#include "Message.hpp"

#include <cstddef>
#include <cstring>
#include <format>
#include <stdexcept>

Message::Message(const std::byte *data, const size_t size) {
  // Validate message size
  if (size > MAXIMUM_MESSAGE_SIZE_BYTES) {
    std::string error_message =
        std::format("Message size ({}) larger than permitted ({})", size,
                    MAXIMUM_MESSAGE_SIZE_BYTES);
    throw std::length_error(error_message);
  } else if (size == 0) {
    throw std::length_error("Message size is 0: no data to copy");
  }

  // Allocate memory for data and copy data to memory
  m_Data = new std::byte[size];
  std::memcpy(m_Data, data, size);
  m_Size = size;
}
