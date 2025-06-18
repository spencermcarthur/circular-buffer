#include "circularbuffer/IWrapper.hpp"

#include <format>
#include <stdexcept>

#include "SharedMemory.hpp"
#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/Indices.hpp"
#include "circularbuffer/Spec.hpp"

namespace CircularBuffer {

IWrapper::IWrapper(const Spec &spec) {
    // Load/map shared memory regions
    m_IndexRegion =
        new SharedMemory(spec.indexSharedMemoryName, sizeof(Iterators));
    m_BufferRegion =
        new SharedMemory(spec.bufferSharedMemoryName, spec.bufferCapacity);

    // Reinterpret indices region as struct and verify
    m_Iters = m_IndexRegion->AsStruct<Iterators>();
    if (m_Iters == nullptr) {
        // Fail
        throw std::runtime_error(std::format(
            "({}:{}) Reinterpretation of index shared memory as struct failed",
            __FILE__, __LINE__));
    }

    // Reinterpret buffer region as span and verify
    m_Buffer = m_BufferRegion->AsSpan<DataT>();
    if (m_Buffer.data() == nullptr || m_Buffer.empty()) {
        // Fail
        throw std::runtime_error(std::format(
            "({}:{}) Reinterpretation of buffer data region as span failed",
            __FILE__, __LINE__));
    }

    // Set local iterator
    m_LocalIter = m_Buffer.begin();
}

IWrapper::~IWrapper() {
    m_Iters = nullptr;

    delete m_BufferRegion;
    delete m_IndexRegion;
}

}  // namespace CircularBuffer
