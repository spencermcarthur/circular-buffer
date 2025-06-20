#include "circularbuffer/Writer.hpp"

#include <atomic>
#include <cstddef>
#include <cstring>
#include <format>
#include <stdexcept>

#include "SemaphoreLock.hpp"
#include "Utils.hpp"
#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/IWrapper.hpp"
#include "circularbuffer/Spec.hpp"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"

namespace CircularBuffer {

Writer::Writer(const Spec& spec)
    : IWrapper(spec), m_SemaphoreLock(MakeWriterSemaphoreName(spec)) {
    SetupSpdlog();
    EnsureSingleton();

    // Writer sets initial shared buffer iterators
    m_State->readIdx.store(0, std::memory_order_release);
    m_State->writeIdx.store(0, std::memory_order_release);

    m_NextElement = m_Buffer.begin();
}

Writer::~Writer() {
    if (!m_SemaphoreLock.Release()) {
        SPDLOG_ERROR("Failed to unlock writer semaphore \"{}\"",
                     m_SemaphoreLock.Name());
    }
}

void Writer::EnsureSingleton() {
    if (!m_SemaphoreLock.Acquire()) {
        throw std::logic_error(std::format(
            "({}:{}) Another writer has locked the semaphore \"{}\"", __FILE__,
            __LINE__, m_SemaphoreLock.Name()));
    }
}

bool Writer::Write(BufferT writeBuffer) {
    // Validate incoming message size
    if (writeBuffer.size_bytes() > MAX_MESSAGE_SIZE) [[unlikely]] {
        SPDLOG_ERROR("Can't write message of size {} B: max size is {} B",
                     writeBuffer.size_bytes(), MAX_MESSAGE_SIZE);
        return false;
    }

    const MessageSizeT msgSize = writeBuffer.size_bytes();
    const size_t totalBytesToWrite = HEADER_SIZE + msgSize;

    /* Handle wraparound
     * If what we need to write is bigger than the remaining
     * space to the end of the buffer, then we want to start writing back at the
     * beginning of the buffer. Want to avoid writing logic to wrap messages
     * around from end to beginning. */
    if (totalBytesToWrite > m_Buffer.size_bytes() - m_LocalIndex) {
        /* Zero out the next message size
         * This signals to readers that they should start reading back at the
         * beginning of the buffer for the next message. */
        std::memset(m_NextElement.base(), '\0', HEADER_SIZE);
        m_LocalIndex = 0;
        m_NextElement = m_Buffer.begin();

        SPDLOG_DEBUG("Wrapping");
    }

    // Advance write index - indicates write in progress
    m_LocalIndex += totalBytesToWrite;
    m_State->writeIdx.store(m_LocalIndex, std::memory_order_release);

    // Write message size to buffer
    std::memcpy(m_NextElement.base(), &msgSize, HEADER_SIZE);

    // Write message to buffer
    std::memcpy(m_NextElement.base() + HEADER_SIZE, writeBuffer.data(),
                msgSize);

    // Advance read index - indicates write complete
    m_State->readIdx.store(m_LocalIndex, std::memory_order_release);

    // Advance next element to next write region
    m_NextElement += totalBytesToWrite;

    SPDLOG_DEBUG("Wrote message of size {} B", msgSize);
    return true;
}

std::string MakeWriterSemaphoreName(const Spec& spec) {
    return spec.dataSharedMemoryName + "-writer";
}

}  // namespace CircularBuffer
