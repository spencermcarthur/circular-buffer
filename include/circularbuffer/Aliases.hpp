#pragma once

#include <cstddef>
#include <span>

namespace CircularBuffer {

using DataT = std::byte;
using BufferT = std::span<DataT>;
using BufferIterT = BufferT::iterator;

static constexpr size_t MAX_MSG_SIZE_BYTES = (1 << 16) - 1;

}  // namespace CircularBuffer
