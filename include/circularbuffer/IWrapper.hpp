#pragma once

#include "Aliases.hpp"
#include "Defines.hpp"
#include "Indices.hpp"
#include "SharedMemory.hpp"
#include "Spec.hpp"

namespace CircularBuffer {

class IWrapper {
public:
    static constexpr size_t HEADER_SIZE = sizeof(MessageSizeT);
    // Buffer should be able to hold at least 2 max-length messages without
    // overwriting
    static constexpr size_t MIN_BUFFER_SIZE =
        2 * (HEADER_SIZE + MAX_MESSAGE_SIZE);

    // No default/copy/move construction
    EXPLICIT_DELETE_CONSTRUCTORS(IWrapper);

protected:
    // Prevent instantiation
    explicit IWrapper(const Spec &spec);
    virtual ~IWrapper();

    // Buffer iterators
    Indices *m_Indices{nullptr};
    // Buffer data
    BufferT m_Buffer;
    // Local cache of index to avoid atomic ops on shared buffer indices as much
    // as possible.
    IndexT m_LocalIndex;

private:
    SharedMemory *m_IndexRegion{nullptr};
    SharedMemory *m_DataRegion{nullptr};
};

}  // namespace CircularBuffer
