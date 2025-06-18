#pragma once

#include <cstddef>
#include <string>

namespace CircularBuffer {

// POD struct for buffer specification
struct Spec {
    // Name of shared memory region for storing buffer indices
    std::string indexSharedMemoryName;
    // Name of shared memory region for storing buffer data
    std::string dataSharedMemoryName;
    // Requested capacity in bytes
    size_t bufferCapacity{0};
};

}  // namespace CircularBuffer
