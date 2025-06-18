#include "circularbuffer/Reader.hpp"

#include <atomic>
#include <cstring>

#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/IWrapper.hpp"
#include "circularbuffer/Spec.hpp"

namespace CircularBuffer {

Reader::Reader(const Spec &spec) : IWrapper(spec) {}

int Reader::Read(BufferT readBuffer) {
    // Check if there's anything to read
    if (m_LocalIter == m_Iters->read.load(std::memory_order_acquire)) {
        // Nothing to read
        return 0;
    }

    // Check if we're at or past the end of the range
    if (m_LocalIter >= m_Buffer.end()) [[unlikely]] {
        m_LocalIter = m_Buffer.begin();
    }

    // Get next message size
    MessageSizeT msgSize;
    std::memcpy(&msgSize, m_LocalIter.base(), sizeof(MessageSizeT));
    m_LocalIter += sizeof(MessageSizeT);

    // TODO: Check for wraparound

    // TODO: Check for overwrite
    // if(overwritten) return -1;

    // Copy next message into readBuffer
    std::memcpy(readBuffer.data(), m_LocalIter.base() + sizeof(int), msgSize);

    // Advance index and data pointer
    m_LocalIter += msgSize;

    return msgSize;
}

}  // namespace CircularBuffer
