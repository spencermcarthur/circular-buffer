#include "circularbuffer/Reader.hpp"

#include <atomic>
#include <cstring>

#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/IWrapper.hpp"
#include "circularbuffer/Spec.hpp"

namespace CircularBuffer {

Reader::Reader(const Spec &spec) : IWrapper(spec) {}

int Reader::Read(BufferT readBuffer) {
    static bool overwritten{false};

    // Don't read any more if we've already been overwritten
    if (overwritten) {
        return -1;
    }

    // If there's nothing to read, return
    if (m_LocalIndex == m_Indices->read.load(std::memory_order_acquire)) {
        return 0;
    }

    // Cache global write index before copy
    const IndexT writeIdxBefore =
        m_Indices->write.load(std::memory_order_acquire);

    // Copy message size
    MessageSizeT msgSize;
    std::memcpy(&msgSize, &m_Buffer[m_LocalIndex], HEADER_SIZE);

    // TODO: Check for wraparound

    // Copy message into read buffer
    std::memcpy(readBuffer.data(), &m_Buffer[m_LocalIndex + HEADER_SIZE],
                msgSize);

    // Load write index after copy
    const IndexT writeIdxAfter =
        m_Indices->read.load(std::memory_order_acquire);

    // TODO: Check for overwrite

    /* Normal overwrite:
     * If we started ahead of the global write index and ended up behind it,
     * then we got overwritten */
    overwritten =
        overwritten || ((m_LocalIndex > writeIdxBefore)      // ahead before
                        && (m_LocalIndex < writeIdxAfter));  // behind after

    /* Wraparound overwrite:
     * If we started behind the write index and are still behind the write
     * index, but it is now behind where it was before we copied, then it
     * wrapped around and overwrote us. */
    overwritten = overwritten ||
                  ((m_LocalIndex < writeIdxAfter)         // behind before
                   && (writeIdxAfter < writeIdxBefore));  // moved "backwards"

    if (overwritten) {
        return -1;
    }

    // Advance index and data pointer
    m_LocalIndex += HEADER_SIZE + msgSize;

    return msgSize;
}

}  // namespace CircularBuffer
