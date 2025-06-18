#pragma once

#include <linux/limits.h>

#include <cstddef>
#include <string>

namespace CircularBuffer {

struct Spec {
    // Name of shared memory region for storing buffer indices
    std::string indexSharedMemoryName;
    // Name of shared memory region for storing buffer data
    std::string bufferSharedMemoryName;
    // Requested capacity in bytes
    size_t bufferCapacity{0};
};

}  // namespace CircularBuffer
