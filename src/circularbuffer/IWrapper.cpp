#include "circularbuffer/IWrapper.hpp"

#include <format>
#include <stdexcept>

#include "SharedMemory.hpp"
#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/Indices.hpp"
#include "circularbuffer/Spec.hpp"

namespace CircularBuffer {

IWrapper::IWrapper(const Spec &spec) {
    // Validate requested buffer size
    if (spec.bufferCapacity < MIN_BUFFER_SIZE) {
        throw std::domain_error(std::format(
            "({}:{}) Buffer size {} is too small: minimum is {}", __FILE__,
            __LINE__, spec.bufferCapacity, MIN_BUFFER_SIZE));
    }

    // Load/map shared memory regions
    m_IndexRegion =
        new SharedMemory(spec.indexSharedMemoryName, sizeof(Indices));
    m_DataRegion =
        new SharedMemory(spec.dataSharedMemoryName, spec.bufferCapacity);

    // Reinterpret index region as struct and verify
    m_Indices = m_IndexRegion->AsStruct<Indices>();
    if (m_Indices == nullptr) {
        // Fail
        throw std::runtime_error(std::format(
            "({}:{}) Reinterpretation of index shared memory as struct failed",
            __FILE__, __LINE__));
    }

    // Reinterpret buffer region as span and verify
    m_Buffer = m_DataRegion->AsSpan<DataT>();
    if (m_Buffer.data() == nullptr || m_Buffer.empty()) {
        // Fail
        throw std::runtime_error(std::format(
            "({}:{}) Reinterpretation of buffer data region as span failed",
            __FILE__, __LINE__));
    }

    // Set local index
    m_LocalIndex = 0;
}

IWrapper::~IWrapper() {
    m_Indices = nullptr;

    delete m_DataRegion;
    delete m_IndexRegion;
}

}  // namespace CircularBuffer
