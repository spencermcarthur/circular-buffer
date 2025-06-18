#pragma once

#include "Aliases.hpp"
#include "Defines.hpp"
#include "IWrapper.hpp"
#include "Spec.hpp"

#include <span>

namespace CircularBuffer {

class Reader : public IWrapper {
public:
  Reader(const Spec &spec) : IWrapper(spec) {}

  // No default/copy/move construction
  DELETE_DEFAULT_CONSTRUCTORS(Reader);

  // Returns number of bytes read, 0 if nothing to read
  int Read(std::span<DataType> readBuffer);
};

} // namespace CircularBuffer
