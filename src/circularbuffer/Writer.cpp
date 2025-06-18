#include "circularbuffer/Writer.hpp"

#include <atomic>
#include <cstring>

#include "circularbuffer/Aliases.hpp"

namespace CircularBuffer {

Writer::Writer(const Spec &spec) : IWrapper(spec) {
    // Writer sets initial shared buffer iterators
    m_Iters->read.store(m_Buffer.begin(), std::memory_order_release);
    m_Iters->write.store(m_Buffer.begin(), std::memory_order_release);
}

void Writer::Write(BufferT writeBuffer) {
    const size_t bufferSize = writeBuffer.size();
    const int totalBytesToWrite = sizeof(int) + bufferSize;
    m_LocalIter += totalBytesToWrite;

    // Advance write index - write in progress
    m_Iters->write.store(m_LocalIter, std::memory_order_release);

    // Write message size to buffer
    std::memcpy(m_NextElement.base(), &bufferSize, sizeof(int));

    // Write message to buffer
    std::memcpy(m_NextElement.base() + sizeof(int), writeBuffer.data(),
                bufferSize);

    // Advance read index - write complete
    m_Iters->write.store(m_LocalIter, std::memory_order_release);

    // Advance next element pointer to next write region
    m_NextElement += totalBytesToWrite;
}

}  // namespace CircularBuffer
