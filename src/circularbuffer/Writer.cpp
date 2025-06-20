#include "circularbuffer/Writer.hpp"

#include <atomic>
#include <cassert>
#include <cstring>
#include <format>
#include <stdexcept>

#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/IWrapper.hpp"
#include "circularbuffer/SemaphoreLock.hpp"
#include "circularbuffer/Spec.hpp"
#include "circularbuffer/Utils.hpp"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"

namespace CircularBuffer {

Writer::Writer(const Spec& spec)
    : IWrapper(spec), m_SemLock(MakeSemName(spec)) {
    SetupSpdlog();
    EnsureSingleton();

    // Writer sets initial shared buffer iterators
    m_State->readIdx.store(0, std::memory_order_release);
    m_State->writeIdx.store(0, std::memory_order_release);
    m_State->seqNum.store(0, std::memory_order_release);

    m_NextElement = m_CircularBuffer.begin();
}

Writer::~Writer() {
    if (!m_SemLock.Release()) {
        SPDLOG_ERROR("Failed to unlock writer semaphore \"{}\"",
                     m_SemLock.Name());
    }
}

bool Writer::Write(BufferT writeBuffer) {
    static_assert(HEADER_SIZE <= sizeof(int));

    // Validate incoming message size
    if (writeBuffer.size_bytes() > MAX_MESSAGE_SIZE) [[unlikely]] {
        SPDLOG_ERROR("Can't write message of size {} B: max size is {} B",
                     writeBuffer.size_bytes(), MAX_MESSAGE_SIZE);
        return false;
    }

    // Compute some values we'll need
    const MessageSizeT msgSize = writeBuffer.size_bytes();
    const int totalBytesToWrite = HEADER_SIZE + msgSize;
    const int spaceToEnd = m_CircularBuffer.size_bytes() - m_LocalIndex;

    // Compute the end of the next write region
    m_LocalIndex += totalBytesToWrite;

    // Enough space to write
    if (totalBytesToWrite <= spaceToEnd) [[likely]] {
        // Advance write index to "reserve" buffer space
        m_State->writeIdx.store(m_LocalIndex, std::memory_order_release);

        // Write message size and data
        std::memcpy(m_NextElement.base(), &msgSize, HEADER_SIZE);
        std::memcpy(m_NextElement.base() + HEADER_SIZE, writeBuffer.data(),
                    msgSize);

        // Advance next write element
        m_NextElement += totalBytesToWrite;
    }
    // Wrapping around
    else {
        // Can fit header
        if (spaceToEnd >= HEADER_SIZE) [[likely]] {
            // Compute index after wraparound
            m_LocalIndex %= m_CircularBuffer.size_bytes();

            // Track bytes left to write
            int bytesRemaining = totalBytesToWrite;
#ifdef DEBUG
            // Track bytes written
            int bytesWritten = 0;
#endif

            // Advance write index to "reserve" buffer space
            m_State->writeIdx.store(m_LocalIndex, std::memory_order_release);

            // Write header and first part of message
            std::memcpy(m_NextElement.base(), &msgSize, HEADER_SIZE);
            std::memcpy(m_NextElement.base() + HEADER_SIZE, writeBuffer.data(),
                        spaceToEnd - HEADER_SIZE);

            // Move pointer to start of buffer
            m_NextElement = m_CircularBuffer.begin();
            // Decrement write countdown
            bytesRemaining -= spaceToEnd;

#ifdef DEBUG
            // Write exactly what was passed to std::memcpy
            bytesWritten += HEADER_SIZE;
            bytesWritten += (spaceToEnd - HEADER_SIZE);
#endif

            // Write rest of message
            std::memcpy(m_NextElement.base(),
                        writeBuffer.data() + bytesRemaining, bytesRemaining);

            m_NextElement += bytesRemaining;

#ifdef DEBUG
            bytesWritten += bytesRemaining;
            // Make sure we wrote the correct amount of bytes
            assert(bytesWritten == totalBytesToWrite);
            // Make sure we tracked remaining bytes correctly
            assert(bytesRemaining == totalBytesToWrite - spaceToEnd);
#endif

            SPDLOG_DEBUG("Wrapped around - split write");
        }
        // Can't fit header - reader will need to do the same calculation to
        // know to wrap around
        else {
            // Move write index to "reserve" buffer space
            m_LocalIndex = msgSize + HEADER_SIZE;
            m_State->writeIdx.store(m_LocalIndex, std::memory_order_release);

            // Move write location to beginning of buffer
            m_NextElement = m_CircularBuffer.begin();

            // Write header and message
            std::memcpy(m_NextElement.base(), &msgSize, HEADER_SIZE);
            std::memcpy(m_NextElement.base() + HEADER_SIZE, writeBuffer.data(),
                        msgSize);

            // Advance next write element
            m_NextElement += totalBytesToWrite;

            SPDLOG_DEBUG("Wrapped around - not enough room for header");
        }
    }

#ifdef DEBUG
    // Make sure next element is set where we put the write index
    assert(m_NextElement.base() == &m_CircularBuffer[m_State->writeIdx]);
#endif

    // Update write sequence number
    m_LocalSeqNum += totalBytesToWrite;
    m_State->seqNum.store(m_LocalSeqNum, std::memory_order_release);

    // Advance read index to indicate that it's safe to read
    m_State->readIdx.store(m_LocalIndex, std::memory_order_release);

    SPDLOG_DEBUG("Wrte message of size {} bytes", msgSize);
    return true;
}

std::string Writer::MakeSemName(const Spec& spec) {
    return spec.dataSharedMemoryName + "-writer";
}

void Writer::EnsureSingleton() {
    if (!m_SemLock.Acquire()) {
        throw std::logic_error(std::format(
            "({}:{}) Another writer has locked the semaphore \"{}\"", __FILE__,
            __LINE__, m_SemLock.Name()));
    }
}

}  // namespace CircularBuffer
