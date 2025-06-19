#pragma once

#include <semaphore.h>

#include <cstddef>
#include <string>

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
    bool Write(BufferT writeBuffer);
    // Compatibility interface
    bool Write(DataT* data, size_t size) { return Write({data, size}); }

private:
    void EnsureSingleton();

    // Pointer to next write location
    IterT m_NextElement;
    // Semaphore lock to ensure only a single reader ever gets instantiated
    SemaphoreLock m_SemaphoreLock;
};

std::string MakeWriterSemaphoreName(const Spec& spec);

}  // namespace CircularBuffer
