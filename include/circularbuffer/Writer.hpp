#pragma once

#include <semaphore.h>

#include <cstddef>

#include "Aliases.hpp"
#include "Defines.hpp"
#include "IWrapper.hpp"
#include "SemaphoreLock.hpp"
#include "Spec.hpp"

namespace CircularBuffer {

class Writer : public IWrapper {
public:
    explicit Writer(const Spec& spec);
    ~Writer() override;

    // No default/copy/move construction
    EXPLICIT_DELETE_CONSTRUCTORS(Writer);

    // Writes data to buffer in shared memory
    void Write(BufferT writeBuffer);
    // Compatibility interface
    void Write(DataT* data, size_t size) { Write({data, size}); }

private:
    void EnsureSingleWriter();

    // Pointer to next write location
    IterT m_NextElement;
    // Semaphore lock to ensure only a single reader ever gets instantiated
    SemaphoreLock m_SemaphoreLock;
};

}  // namespace CircularBuffer
