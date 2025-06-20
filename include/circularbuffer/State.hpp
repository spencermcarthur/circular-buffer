#pragma once

#include <atomic>

#include "Aliases.hpp"

namespace CircularBuffer {

// POD struct for maintaining buffer state in shared memory
struct State {
    // Cacheline alignement needed to avoid false sharing
    alignas(__CACHELINE_SIZE_BYTES) std::atomic<IndexT> readIdx;
    alignas(__CACHELINE_SIZE_BYTES) std::atomic<IndexT> writeIdx;
};

}  // namespace CircularBuffer
