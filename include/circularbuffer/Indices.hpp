#pragma once

#include <atomic>

#include "Aliases.hpp"

namespace CircularBuffer {

struct Iterators {
  // Cacheline alignement needed to avoid false sharing
  alignas(CACHELINE_SIZE_BYTES) std::atomic<BufferIterT> read;
  alignas(CACHELINE_SIZE_BYTES) std::atomic<BufferIterT> write;
};

} // namespace CircularBuffer
