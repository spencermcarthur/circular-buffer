#pragma once

#include <cstddef>
#include <linux/limits.h>
#include <string>

namespace CircularBuffer {

struct Spec {
  // Name for shared memory region storing buffer indices
  std::string indexSharedMemoryName;
  // Name for shared memory region storing buffer data
  std::string dataSharedMemoryName;
  // Requested capacity in bytes
  size_t bufferCapacity{0};
};

} // namespace CircularBuffer
