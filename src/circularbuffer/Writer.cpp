#include "circularbuffer/Writer.hpp"

#include <atomic>
#include <cstring>

#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/IWrapper.hpp"
#include "circularbuffer/Spec.hpp"

namespace CircularBuffer {

Writer::Writer(const Spec &spec) : IWrapper(spec) {
    // Writer sets initial shared buffer iterators
    m_Iters->read.store(m_Buffer.begin(), std::memory_order_release);
    m_Iters->write.store(m_Buffer.begin(), std::memory_order_release);

    m_NextElement = m_Buffer.begin();
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
