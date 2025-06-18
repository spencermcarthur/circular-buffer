#include "circularbuffer/Reader.hpp"

#include <atomic>
#include <cstring>

#include "circularbuffer/Aliases.hpp"

namespace CircularBuffer {

Reader::Reader(const Spec &spec) : IWrapper(spec) {}

int Reader::Read(BufferT readBuffer) {
    // Check if there's anything to read
    if (m_LocalIter == m_Iters->read.load(std::memory_order_acquire)) {
        // Nothing to read
        return 0;
    }

    // Get next message size
    int size;
    std::memcpy(&size, m_NextElement.base(), sizeof(int));

#pragma GCC warning "TODO: check for overwrite"

    // Copy next message into readBuffer
    std::memcpy(readBuffer.data(), m_NextElement.base() + sizeof(int), size);

    // Advance index and data pointer
    const int totalBytesRead = sizeof(int) + size;
    m_LocalIter += totalBytesRead;
    m_NextElement += totalBytesRead;

    return size;
}

}  // namespace CircularBuffer
