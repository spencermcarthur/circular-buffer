#pragma once

#include <cstddef>

#include "Aliases.hpp"
#include "Macros.hpp"
#include "IWrapper.hpp"
#include "Spec.hpp"

namespace CircularBuffer {

class Reader : public IWrapper {
public:
    explicit Reader(const Spec &spec);

    // No default/copy/move construction
    EXPLICIT_DELETE_CONSTRUCTORS(Reader);

    // Returns positive int if buffer read-from successfully, or 0 if there is
    // no data to read. Returns -1 if the read buffer is too small. Returns
    // `INT_MIN` if the Reader got overwritten by the Writer.
    int Read(BufferT readBuffer);
    // Compatibility interface
    int Read(DataT *data, size_t size) { return Read({data, size}); }

private:
    IndexT m_CachedReadIdx;
    IndexT m_CachedWriteIdx;
};

}  // namespace CircularBuffer
