#pragma once

#include "Aliases.hpp"
#include "Defines.hpp"
#include "Indices.hpp"
#include "SharedMemory.hpp"
#include "Spec.hpp"

namespace CircularBuffer {

class IWrapper {
public:
  IWrapper(const Spec &spec);
  virtual ~IWrapper();

  // No default/copy/move construction
  EXPLICIT_DELETE_CONSTRUCTORS(IWrapper);

protected:
  Iterators *m_Iters{nullptr};
  BufferT m_Buffer;

  BufferIterT m_LocalIter;
  BufferIterT m_NextElement;

private:
  SharedMemory *m_IndexRegion{nullptr};
  SharedMemory *m_BufferRegion{nullptr};
};

} // namespace CircularBuffer
