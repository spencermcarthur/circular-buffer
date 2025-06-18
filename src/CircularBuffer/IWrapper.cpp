#include "CircularBuffer/IWrapper.hpp"
#include "CircularBuffer/Indices.hpp"

namespace CircularBuffer {

IWrapper::IWrapper(const Spec &spec) {
  m_IndexRegion = new SharedMemoryRegion(spec.indexSharedMemoryName.c_str(),
                                         sizeof(Indices));
  m_DataRegion = new SharedMemoryRegion(spec.dataSharedMemoryName.c_str(),
                                        spec.bufferCapacity);

  m_BufferIndices = m_IndexRegion->AsStruct<Indices>();
  m_BufferData = m_DataRegion->AsSpan<DataType>();
}

IWrapper::~IWrapper() {
  m_BufferIndices = nullptr;

  delete m_DataRegion;
  delete m_IndexRegion;
}

} // namespace CircularBuffer
