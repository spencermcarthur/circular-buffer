#include "circularbuffer/Reader.hpp"

#include <atomic>
#include <climits>
#include <cstring>

#include "Utils.hpp"
#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/IWrapper.hpp"
#include "circularbuffer/Spec.hpp"
#include "spdlog/spdlog.h"

namespace CircularBuffer {

Reader::Reader(const Spec& spec) : IWrapper(spec) { SetupSpdlog(); }

int Reader::Read(BufferT readBuffer) {
    static bool overwritten{false};

    // Don't read any more if we've already been overwritten
    if (overwritten) {
        return INT_MIN;
    }

    // If there's nothing to read, return
    if (m_LocalIndex == m_State->readIdx.load(std::memory_order_acquire)) {
        return 0;
    }

    // Copy message size
    MessageSizeT msgSize;
    std::memcpy(&msgSize, &m_Buffer[m_LocalIndex], HEADER_SIZE);

    // Buffer not big enough
    if (msgSize > readBuffer.size_bytes()) {
        SPDLOG_ERROR("Read buffer too small: {} B vs message size of {} B",
                     readBuffer.size_bytes(), msgSize);
        return -1;
    }

    // Check for wraparound. If writer zeroed current header and moved read
    // pointer, then we know it "overflowed" to the beginning.
    if (msgSize == 0) {
        m_LocalIndex = 0;
        std::memcpy(&msgSize, &m_Buffer[m_LocalIndex], HEADER_SIZE);
    }

    // Cache global write index before copy
    const IndexT writeIdxBefore =
        m_State->writeIdx.load(std::memory_order_acquire);

    // Copy message into read buffer
    std::memcpy(readBuffer.data(), &m_Buffer[m_LocalIndex + HEADER_SIZE],
                msgSize);

    // Load write index after copy
    const IndexT writeIdxAfter =
        m_State->writeIdx.load(std::memory_order_acquire);

    // Overwrite detection
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
        SPDLOG_CRITICAL("Got overwritten");
        return INT_MIN;
    }

    // Advance index and data pointer
    m_LocalIndex += HEADER_SIZE + msgSize;

    SPDLOG_DEBUG("Read message of size {} bytes", msgSize);
    return msgSize;
}

}  // namespace CircularBuffer
