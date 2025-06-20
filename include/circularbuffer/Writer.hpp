#pragma once

#include <semaphore.h>

#include <cstddef>
#include <string>

#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/IWrapper.hpp"
#include "circularbuffer/Macros.hpp"
#include "circularbuffer/SemaphoreLock.hpp"
#include "circularbuffer/Spec.hpp"

namespace CircularBuffer {

class Writer : public IWrapper {
public:
    explicit Writer(const Spec& spec);
    ~Writer() override;

    // No default/copy/move construction
    CB_EXPLICIT_DELETE_CONSTRUCTORS(Writer);

    // Writes data to buffer in shared memory
    bool Write(BufferT writeBuffer);
    // Compatibility interface
    bool Write(DataT* data, size_t size) { return Write({data, size}); }

    static std::string MakeSemName(const Spec& spec);

private:
    void EnsureSingleton();

    // Pointer to next write location
    IterT m_NextElement;
    // Semaphore lock to ensure only a single reader ever gets instantiated
    SemaphoreLock m_SemLock;
};

}  // namespace CircularBuffer
