#ifndef CIRCULARBUFFER_HPP
#define CIRCULARBUFFER_HPP

#include <cstddef>

template <typename MessageType, size_t BufferSize> class CircularBuffer {
public:
  CircularBuffer() = default;

  // No copy
  CircularBuffer(const CircularBuffer &) = delete;
  CircularBuffer &operator=(const CircularBuffer &) = delete;

  // Move-enabled
  CircularBuffer(CircularBuffer &&) = default;
  CircularBuffer &operator=(CircularBuffer &&) = default;

private:
  MessageType m_Data[BufferSize];
};

#endif // CIRCULARBUFFER_HPP
