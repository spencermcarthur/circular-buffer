#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstddef>

class Message {
public:
  Message(const std::byte *data, const size_t size);

  const std::byte *data() const { return m_Data; }
  size_t size() const { return m_Size; }

private:
  // Message size < 2^16 bytes
  static constexpr size_t MAXIMUM_MESSAGE_SIZE_BYTES = 65535;

  std::byte *m_Data{nullptr};
  size_t m_Size{0};
};

#endif // MESSAGE_HPP