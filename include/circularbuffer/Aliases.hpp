#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace CircularBuffer {

using DataT = std::byte;
using BufferT = std::span<DataT>;
using IndexT = size_t;
using IterT = BufferT::iterator;
using MessageSizeT = uint16_t;

static constexpr size_t MAX_MESSAGE_SIZE = __MAX_MESSAGE_SIZE_BYTES;
static constexpr size_t HEADER_SIZE = sizeof(MessageSizeT);

}  // namespace CircularBuffer
