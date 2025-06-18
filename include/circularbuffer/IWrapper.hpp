#pragma once

#include "Aliases.hpp"
#include "Defines.hpp"
#include "Indices.hpp"
#include "SharedMemory.hpp"
#include "Spec.hpp"

namespace CircularBuffer {

class IWrapper {
public:
    // No default/copy/move construction
    EXPLICIT_DELETE_CONSTRUCTORS(IWrapper);

protected:
    explicit IWrapper(const Spec &spec);
    virtual ~IWrapper();

    // Buffer iterators
    Iterators *m_Iters{nullptr};
    // Buffer data
    BufferT m_Buffer;
    // Local iterator to avoid atomic ops on buffer iterators as much as
    // possible. Mostly benefits the writer.
    BufferIterT m_LocalIter;

private:
    SharedMemory *m_IndexRegion{nullptr};
    SharedMemory *m_BufferRegion{nullptr};
};

}  // namespace CircularBuffer
