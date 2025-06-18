#pragma once

#include <cstddef>

#include "Aliases.hpp"
#include "Defines.hpp"
#include "IWrapper.hpp"
#include "Spec.hpp"

namespace CircularBuffer {

class Reader : public IWrapper {
public:
    explicit Reader(const Spec &spec);

    // No default/copy/move construction
    EXPLICIT_DELETE_CONSTRUCTORS(Reader);

    // Returns number of bytes read, 0 if nothing to read
    int Read(BufferT readBuffer);
    // Compatibility interface
    int Read(DataT *data, size_t size) { return Read({data, size}); }
};

}  // namespace CircularBuffer
