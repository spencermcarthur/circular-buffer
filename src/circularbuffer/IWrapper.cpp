#include "circularbuffer/IWrapper.hpp"
#include "circularbuffer/Indices.hpp"

#include <format>
#include <stdexcept>

namespace CircularBuffer {

IWrapper::IWrapper(const Spec &spec) {
  m_IndexRegion = new SharedMemoryRegion(spec.indexSharedMemoryName.c_str(),
                                         sizeof(Indices));
  m_DataRegion = new SharedMemoryRegion(spec.dataSharedMemoryName.c_str(),
                                        spec.bufferCapacity);

  // Reinterpret region as struct
  m_Indices = m_IndexRegion->AsStruct<Indices>();
  if (m_Indices == nullptr) {
    // Fail
    throw std::runtime_error(std::format(
        "({}:{}) Reinterpretation of index shared memory as struct failed",
        __FILE__, __LINE__));
  }

  // Reinterpret region as span
  m_Data = m_DataRegion->AsSpan<DataType>();
  if (m_Data.data() == nullptr || m_Data.size() == 0) {
    // Fail
    throw std::runtime_error(std::format(
        "({}:{}) Reinterpretation of buffer data region as span failed",
        __FILE__, __LINE__));
  }

  m_NextElement = m_Data.data();
  m_LocalIdx = 0;
}

IWrapper::~IWrapper() {
  m_Indices = nullptr;

  delete m_DataRegion;
  delete m_IndexRegion;
}

} // namespace CircularBuffer
