#pragma once

#include <cstddef>

#include "Aliases.hpp"
#include "Defines.hpp"
#include "IWrapper.hpp"
#include "Spec.hpp"

namespace CircularBuffer {

class Writer : public IWrapper {
public:
    Writer(const Spec& spec);

    // No default/copy/move construction
    EXPLICIT_DELETE_CONSTRUCTORS(Writer);

    // Writes data to buffer in shared memory
    void Write(BufferT writeBuffer);
    // Compatibility interface
    void Write(DataT* data, size_t size) { Write({data, size}); }

private:
    // Pointer to next write location
    BufferIterT m_NextElement;
};

}  // namespace CircularBuffer
