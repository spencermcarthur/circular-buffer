#pragma once

#include "Aliases.hpp"
#include "Defines.hpp"
#include "IWrapper.hpp"
#include "Spec.hpp"

namespace CircularBuffer {

class Reader : public IWrapper {
public:
    Reader(const Spec &spec);

    // No default/copy/move construction
    EXPLICIT_DELETE_CONSTRUCTORS(Reader);

    // Returns number of bytes read, 0 if nothing to read
    int Read(BufferT readBuffer);
};

}  // namespace CircularBuffer
