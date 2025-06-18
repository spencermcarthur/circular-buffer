#pragma once

#include "Aliases.hpp"
#include "Defines.hpp"
#include "IWrapper.hpp"
#include "Spec.hpp"

#include <span>

namespace CircularBuffer {

class Writer : public IWrapper {
public:
  Writer(const Spec &spec) : IWrapper(spec) {}

  // No default/copy/move construction
  DELETE_DEFAULT_CONSTRUCTORS(Writer);

  // Writes data to buffer in shared memory
  void Write(std::span<DataType> writeBuffer);
};

} // namespace CircularBuffer
