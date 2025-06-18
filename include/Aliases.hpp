#pragma once

#include <cstddef>
#include <cstdint>

using IndexType = int64_t;
using DataType = std::byte;

static constexpr size_t MAX_MSG_SIZE_BYTES = (1 << 16) - 1;
