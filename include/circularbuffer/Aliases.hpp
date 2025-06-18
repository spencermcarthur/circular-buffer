#pragma once

#include <cstddef>
#include <limits>
#include <span>

namespace CircularBuffer {

using DataT = std::byte;
using BufferT = std::span<DataT>;
using BufferIterT = BufferT::iterator;
using MessageSizeT = unsigned short;

// (2^16)-1 = 65535 bytes
static constexpr MessageSizeT MAX_MESSAGE_SIZE_BYTES =
    std::numeric_limits<MessageSizeT>::max();

}  // namespace CircularBuffer
