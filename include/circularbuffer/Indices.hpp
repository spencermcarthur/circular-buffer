#pragma once

#include <atomic>

#include "Aliases.hpp"

namespace CircularBuffer {

// POD struct for maintaining buffer r/w indices in shared memory
struct Iterators {
    // Cacheline alignement needed to avoid false sharing
    alignas(CACHELINE_SIZE_BYTES) std::atomic<BufferIterT> read;
    alignas(CACHELINE_SIZE_BYTES) std::atomic<BufferIterT> write;
};

}  // namespace CircularBuffer
