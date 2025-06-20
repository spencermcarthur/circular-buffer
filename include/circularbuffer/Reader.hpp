#pragma once

#include <cstddef>

#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/IWrapper.hpp"
#include "circularbuffer/Macros.hpp"
#include "circularbuffer/Spec.hpp"

namespace CircularBuffer {

class Reader : public IWrapper {
public:
    explicit Reader(const Spec &spec);

    // No default/copy/move construction
    CB_EXPLICIT_DELETE_CONSTRUCTORS(Reader);

    // Returns positive int if buffer read-from successfully, or 0 if there is
    // no data to read. Returns -1 if the read buffer is too small. Returns
    // `INT_MIN` if the Reader got overwritten by the Writer.
    int Read(BufferT readBuffer);
    // Compatibility interface
    int Read(DataT *data, size_t size) { return Read({data, size}); }
};

}  // namespace CircularBuffer
