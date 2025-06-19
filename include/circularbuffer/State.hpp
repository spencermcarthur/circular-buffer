#pragma once

#include <atomic>

#include "Aliases.hpp"

namespace CircularBuffer {

// POD struct for maintaining buffer state in shared memory
struct State {
    // Cacheline alignement needed to avoid false sharing
    alignas(CACHELINE_SIZE_BYTES) std::atomic<IndexT> readIdx;
    alignas(CACHELINE_SIZE_BYTES) std::atomic<IndexT> writeIdx;
};

}  // namespace CircularBuffer
