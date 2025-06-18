#include "circularbuffer/Reader.hpp"
#include "circularbuffer/Aliases.hpp"

#include <atomic>
#include <cstring>

namespace CircularBuffer {

int Reader::Read(std::span<DataType> readBuffer) {
  // Check if there's anything to read
  if (m_LocalIdx == m_Indices->read.load(std::memory_order_acquire)) {
    // Nothing to read
    return 0;
  }

  // Get next message size
  int size;
  std::memcpy(&size, m_NextElement, sizeof(int));

#pragma GCC warning "TODO: check for overwrite"

  // Copy next message into readBuffer
  std::memcpy(readBuffer.data(), m_NextElement + sizeof(int), size);

  // Advance index and data pointer
  const int totalBytesRead = sizeof(int) + size;
  m_LocalIdx += totalBytesRead;
  m_NextElement += totalBytesRead;

  return size;
}

} // namespace CircularBuffer
