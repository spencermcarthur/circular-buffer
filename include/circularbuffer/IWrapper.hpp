#pragma once

#include "Aliases.hpp"
#include "Defines.hpp"
#include "SharedMemory.hpp"
#include "Spec.hpp"
#include "State.hpp"

namespace CircularBuffer {

class IWrapper {
public:
    // No default/copy/move construction
    EXPLICIT_DELETE_CONSTRUCTORS(IWrapper);

protected:
    // Prevent instantiation
    explicit IWrapper(const Spec &spec);
    virtual ~IWrapper();

    // Buffer state
    State *m_State{nullptr};
    // Buffer data
    BufferT m_Buffer;
    // Local cache of index to avoid atomic ops on shared buffer indices as much
    // as possible.
    IndexT m_LocalIndex;

private:
    SharedMemory *m_StateRegion{nullptr};
    SharedMemory *m_DataRegion{nullptr};
};

}  // namespace CircularBuffer
