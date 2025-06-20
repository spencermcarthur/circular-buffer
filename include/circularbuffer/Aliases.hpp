#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "circularbuffer/Macros.hpp"

namespace CircularBuffer {

using DataT = std::byte;
using BufferT = std::span<DataT>;
using IndexT = uint64_t;
using IterT = BufferT::iterator;
using MessageSizeT = int32_t;
using SeqNumT = uint64_t;

static constexpr int CACHELINE_SIZE = CB_CACHELINE_SIZE_BYTES;
static constexpr int MAX_MESSAGE_SIZE = CB_MAX_MESSAGE_SIZE_BYTES;
static constexpr int HEADER_SIZE = sizeof(MessageSizeT);

}  // namespace CircularBuffer
