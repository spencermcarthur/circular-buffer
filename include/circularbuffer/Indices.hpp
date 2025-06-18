#pragma once

#include <atomic>

#include "Aliases.hpp"

namespace CircularBuffer {

// POD struct for maintaining buffer r/w indices in shared memory
struct Indices {
    // Cacheline alignement needed to avoid false sharing
    alignas(CACHELINE_SIZE_BYTES) std::atomic<IndexT> read;
    alignas(CACHELINE_SIZE_BYTES) std::atomic<IndexT> write;
};

}  // namespace CircularBuffer
