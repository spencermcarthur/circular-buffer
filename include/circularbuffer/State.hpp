#pragma once

#include <atomic>

#include "circularbuffer/Aliases.hpp"

namespace CircularBuffer {

// POD struct for maintaining buffer state in shared memory
struct State {
    // Cacheline alignement needed to avoid false sharing
    alignas(CACHELINE_SIZE) std::atomic<IndexT> readIdx;
    alignas(CACHELINE_SIZE) std::atomic<IndexT> writeIdx;
    alignas(CACHELINE_SIZE) std::atomic<SeqNumT> seqNum;
};

}  // namespace CircularBuffer
