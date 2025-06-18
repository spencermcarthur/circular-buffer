#pragma once

#include <atomic>

#include "Aliases.hpp"
#include "Defines.hpp"

namespace CircularBuffer {

struct Indices {
  // Cacheline alignement needed to avoid false sharing
  alignas(CACHELINE_SIZE_BYTES) std::atomic<IndexType> read;
  alignas(CACHELINE_SIZE_BYTES) std::atomic<IndexType> write;
};

} // namespace CircularBuffer
