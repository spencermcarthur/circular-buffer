#include "circularbuffer/Reader.hpp"

#include <atomic>
#ifdef DEBUG
#include <cassert>
#endif
#include <climits>
#include <cstddef>
#include <cstring>

#include "Utils.hpp"
#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/IWrapper.hpp"
#include "circularbuffer/Spec.hpp"
#include "spdlog/spdlog.h"

namespace CircularBuffer {

Reader::Reader(const Spec& spec) : IWrapper(spec) { SetupSpdlog(); }

int Reader::Read(BufferT readBuffer) {
    static_assert(HEADER_SIZE <= sizeof(int));

    // Check if there's data to read
    if (m_LocalIndex == m_State->readIdx.load(std::memory_order_acquire)) {
        // Nothing to read
        return 0;
    }

    // Overwrite detection: How far behind in sequence number are we?
    SeqNumT lag =
        m_State->seqNum.load(std::memory_order_acquire) - m_LocalSeqNum;
    if (lag > m_CircularBuffer.size_bytes()) [[unlikely]] {
        // Overwritten
        SPDLOG_CRITICAL("Overwrite detected: writer is {} bytes ahead of me",
                        lag);
        return INT_MIN;
    }

    // Space to end of buffer
    const int spaceToEnd = m_CircularBuffer.size_bytes() - m_LocalIndex;

    MessageSizeT msgSize;

    // Header can fit
    if (spaceToEnd >= HEADER_SIZE) [[likely]] {
        // Read message size
        std::memcpy(&msgSize, &m_CircularBuffer[m_LocalIndex], HEADER_SIZE);

        // Validate message size
        if (msgSize < 0 || msgSize > MAX_MESSAGE_SIZE) [[unlikely]] {
            SPDLOG_CRITICAL("Message size error: {} is invalid", msgSize);
            return -1;
        }

#ifdef DEBUG
        // Track bytes read/remaining as we read
        int totalBytesRead = HEADER_SIZE;
        int remainingBytes = msgSize;
#endif

        // Check read buffer is big enough
        if (static_cast<size_t>(msgSize) > readBuffer.size_bytes())
            [[unlikely]] {
            SPDLOG_ERROR("Read buffer too small: {} B vs message size of {} B",
                         readBuffer.size_bytes(), msgSize);
            return -1;
        }

        // Compute total bytes we need to read
        const int totalBytesToRead = HEADER_SIZE + msgSize;

        // Message fits - can read like normal
        if (totalBytesToRead <= spaceToEnd) [[likely]] {
            // Read buffer data and shift pointer
            std::memcpy(readBuffer.data(),
                        &m_CircularBuffer[m_LocalIndex + HEADER_SIZE], msgSize);
            m_LocalIndex += totalBytesToRead;

#ifdef DEBUG
            totalBytesRead += msgSize;
            remainingBytes -= msgSize;
#endif
        }
        // Message wraps - need to split read
        else {
            int msgBytesRead = 0;

            // Read first part and shift pointer to beginning of buffer
            std::memcpy(readBuffer.data(),
                        &m_CircularBuffer[m_LocalIndex + HEADER_SIZE],
                        spaceToEnd - HEADER_SIZE);
            m_LocalIndex = 0;
            msgBytesRead += (spaceToEnd - HEADER_SIZE);

#ifdef DEBUG
            totalBytesRead += (spaceToEnd - HEADER_SIZE);
            remainingBytes -= (spaceToEnd - HEADER_SIZE);
#endif

            // Read second part and shift pointer again
            const int bytesLeft = totalBytesToRead - spaceToEnd;
            std::memcpy(readBuffer.data() + msgBytesRead,
                        &m_CircularBuffer[m_LocalIndex], bytesLeft);
            m_LocalIndex += bytesLeft;

#ifdef DEBUG
            totalBytesRead += bytesLeft;
            remainingBytes -= bytesLeft;
#endif

            SPDLOG_DEBUG("Detected wraparound - split read");
        }

        m_LocalSeqNum += totalBytesToRead;

#ifdef DEBUG
        assert(totalBytesRead == totalBytesToRead);
        assert(remainingBytes == 0);
#endif
    }
    // Header can't fit - writer will have wrapped around
    else {
        // Wrap around to start of buffer
        m_LocalIndex = 0;

        // Read message size
        std::memcpy(&msgSize, &m_CircularBuffer[m_LocalIndex], HEADER_SIZE);

        // Validate message size
        if (msgSize < 0 || msgSize > MAX_MESSAGE_SIZE) [[unlikely]] {
            SPDLOG_CRITICAL("Message size error: {} is invalid", msgSize);
            return -1;
        }

        // Check read buffer is big enough
        if (static_cast<size_t>(msgSize) > readBuffer.size_bytes())
            [[unlikely]] {
            SPDLOG_ERROR("Read buffer too small: {} B vs message size of {} B",
                         readBuffer.size_bytes(), msgSize);
            return -1;
        }

        // Read message
        std::memcpy(readBuffer.data(),
                    &m_CircularBuffer[m_LocalIndex + HEADER_SIZE], msgSize);

        // Move pointers
        const int totalBytesRead = HEADER_SIZE + msgSize;
        m_LocalIndex += totalBytesRead;
        m_LocalSeqNum += totalBytesRead;

        SPDLOG_DEBUG("Detected wraparound - header can't fit");
    }

    // Overwrite detection: How far behind in sequence number are we?
    lag = m_State->seqNum.load(std::memory_order_acquire) - m_LocalSeqNum;
    if (lag > m_CircularBuffer.size_bytes()) [[unlikely]] {
        // Overwritten
        SPDLOG_CRITICAL("Overwrite detected: writer is {} bytes ahead of me",
                        lag);
        return INT_MIN;
    }

    SPDLOG_DEBUG("Read message of size {} bytes", msgSize);
    return msgSize;
}

}  // namespace CircularBuffer
