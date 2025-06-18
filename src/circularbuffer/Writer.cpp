#include "circularbuffer/Writer.hpp"

#include <atomic>
#include <cstddef>
#include <cstring>
#include <format>
#include <iostream>
#include <stdexcept>

#include "SemaphoreLock.hpp"
#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/IWrapper.hpp"
#include "circularbuffer/Spec.hpp"

namespace CircularBuffer {

Writer::Writer(const Spec& spec)
    : IWrapper(spec), m_SemaphoreLock(spec.dataSharedMemoryName + "-writer") {
    EnsureSingleWriter();

    // Writer sets initial shared buffer iterators
    m_Indices->read.store(0, std::memory_order_release);
    m_Indices->write.store(0, std::memory_order_release);

    m_NextElement = m_Buffer.begin();
}

Writer::~Writer() {
    if (!m_SemaphoreLock.Release()) {
        std::cerr << std::format("({}:{}) Failed to unlock writer semaphore\n",
                                 __FILE__, __LINE__);
    }
}

void Writer::EnsureSingleWriter() {
    if (!m_SemaphoreLock.Acquire()) {
        throw std::logic_error(
            std::format("({}:{}) Another writer has locked the semaphore",
                        __FILE__, __LINE__));
    }
}

void Writer::Write(BufferT writeBuffer) {
    const size_t msgSize = writeBuffer.size_bytes();

    // Validate incoming message size
    if (msgSize > MAX_MESSAGE_SIZE) [[unlikely]] {
        // Log this? Error?
        return;
    }

    const size_t totalBytesToWrite = HEADER_SIZE + msgSize;

    /* Handle wraparound
     * If what we need to write is bigger than the remaining
     * space to the end of the buffer, then we want to start writing back at the
     * beginning of the buffer. Want to avoid writing logic to wrap messages
     * around from end to beginning. */
    if (totalBytesToWrite > m_Buffer.size_bytes() - m_LocalIndex) [[unlikely]] {
        /* Zero out the next message size
         * This signals to readers that they should start reading back at the
         * beginning of the buffer for the next message. */
        std::memset(m_NextElement.base(), '\0', HEADER_SIZE);
        m_LocalIndex = 0;
        m_NextElement = m_Buffer.begin();
    }

    // Advance write index - indicates write in progress
    m_LocalIndex += totalBytesToWrite;
    m_Indices->write.store(m_LocalIndex, std::memory_order_release);

    // Write message size to buffer
    std::memcpy(m_NextElement.base(), &msgSize, HEADER_SIZE);

    // Write message to buffer
    std::memcpy(m_NextElement.base() + HEADER_SIZE, writeBuffer.data(),
                msgSize);

    // Advance read index - indicates write complete
    m_Indices->read.store(m_LocalIndex, std::memory_order_release);

    // Advance next element to next write region
    m_NextElement += totalBytesToWrite;
}

}  // namespace CircularBuffer
