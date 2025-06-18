#pragma once

#include "Aliases.hpp"
#include "Defines.hpp"
#include "Indices.hpp"
#include "SharedMemoryRegion.hpp"
#include "Spec.hpp"

#include <span>

namespace CircularBuffer {

class IWrapper {
public:
  IWrapper(const Spec &spec);
  virtual ~IWrapper();

  // No default/copy/move construction
  DELETE_DEFAULT_CONSTRUCTORS(IWrapper);

protected:
  Indices *m_Indices{nullptr};
  std::span<DataType> m_Data{};

  IndexType m_LocalIdx{0};
  DataType *m_NextElement{nullptr};

private:
  SharedMemoryRegion *m_IndexRegion{nullptr};
  SharedMemoryRegion *m_DataRegion{nullptr};
};

} // namespace CircularBuffer
