#include "circularbuffer/Writer.hpp"

#include <atomic>
#include <cstring>

namespace CircularBuffer {

void Writer::Write(std::span<DataType> writeBuffer) {
  const size_t bufferSize = writeBuffer.size();

  const int totalBytesToWrite = sizeof(int) + bufferSize;
  m_LocalIdx += totalBytesToWrite;

#pragma GCC warning "TODO: handle wrap-around"

  // Advance write index - write in progress
  m_Indices->write.store(m_LocalIdx, std::memory_order_release);

  // Copy message size
  std::memcpy(m_NextElement, &bufferSize, sizeof(int));
  // Copy message
  std::memcpy(m_NextElement + sizeof(int), writeBuffer.data(), bufferSize);

  // Advance read index - write complete
  m_Indices->write.store(m_LocalIdx, std::memory_order_release);

  // Advance next element pointer to next write region
  m_NextElement += totalBytesToWrite;
}

} // namespace CircularBuffer
