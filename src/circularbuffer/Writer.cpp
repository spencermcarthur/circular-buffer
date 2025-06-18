#include "circularbuffer/Writer.hpp"

#include <atomic>
#include <cstring>
#include <format>
#include <iostream>
#include <stdexcept>

#include "SemLock.hpp"
#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/IWrapper.hpp"
#include "circularbuffer/Spec.hpp"

namespace CircularBuffer {

Writer::Writer(const Spec& spec)
    : IWrapper(spec), m_SemLock(spec.bufferSharedMemoryName + "-writer") {
    EnsureSingleWriter();

    // Writer sets initial shared buffer iterators
    m_Iters->read.store(m_Buffer.begin(), std::memory_order_release);
    m_Iters->write.store(m_Buffer.begin(), std::memory_order_release);

    m_NextElement = m_Buffer.begin();
}

Writer::~Writer() {
    if (!m_SemLock.Release()) {
        std::cerr << std::format("({}:{}) Failed to unlock writer semaphore\n",
                                 __FILE__, __LINE__);
    }
}

void Writer::EnsureSingleWriter() {
    if (!m_SemLock.Acquire()) {
        throw std::runtime_error(
            std::format("({}:{}) Another writer has locked the semaphore",
                        __FILE__, __LINE__));
    }
}

void Writer::Write(BufferT writeBuffer) {
    // Validate message size
    if (writeBuffer.size() > MAX_MESSAGE_SIZE_BYTES) {
        // Do something here?
        return;
    }

    const MessageSizeT msgSize = writeBuffer.size();

    // TODO: Check for wraparound

    // Advance local iterator
    const int totalBytesToWrite = sizeof(MessageSizeT) + msgSize;
    m_LocalIter += totalBytesToWrite;

    // Advance write index - write in progress
    m_Iters->write.store(m_LocalIter, std::memory_order_release);

    // Write message size to buffer
    std::memcpy(m_NextElement.base(), &msgSize, sizeof(MessageSizeT));

    // Write message to buffer
    std::memcpy(m_NextElement.base() + sizeof(MessageSizeT), writeBuffer.data(),
                msgSize);

    // Advance read index - write complete
    m_Iters->read.store(m_LocalIter, std::memory_order_release);

    // Advance next element to next write region
    m_NextElement += totalBytesToWrite;
}

}  // namespace CircularBuffer
