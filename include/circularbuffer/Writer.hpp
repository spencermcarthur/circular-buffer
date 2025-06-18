#pragma once

#include "Aliases.hpp"
#include "Defines.hpp"
#include "IWrapper.hpp"
#include "Spec.hpp"

namespace CircularBuffer {

class Writer : public IWrapper {
public:
  Writer(const Spec &spec);

  // No default/copy/move construction
  EXPLICIT_DELETE_CONSTRUCTORS(Writer);

  // Writes data to buffer in shared memory
  void Write(BufferT writeBuffer);
};

} // namespace CircularBuffer
