#pragma once

#include <atomic>

#include "Aliases.hpp"
#include "Defines.hpp"

namespace CircularBuffer {

struct Indices {
  // Cacheline alignement needed to avoid false sharing
  alignas(CACHE_LINE_SIZE) std::atomic<IndexType> m_ReadIdx;
  alignas(CACHE_LINE_SIZE) std::atomic<IndexType> m_WriteIdx;
};

} // namespace CircularBuffer
