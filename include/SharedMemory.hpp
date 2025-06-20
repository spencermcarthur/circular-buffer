#pragma once

#include <linux/limits.h>

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "Defines.hpp"
#include "SemaphoreLock.hpp"

// Class for managing Linux shared memory
class SharedMemory {
    // Needed for putting the ref counter on its own cacheline
    static constexpr size_t DATA_OFFSET_BYTES = __CACHELINE_SIZE_BYTES;

public:
    // See "DESCRIPTION" at
    // https://man7.org/linux/man-pages/man3/shm_open.3.html
    static constexpr size_t MAX_NAME_LEN = NAME_MAX;
    // Arbitrary 500 MiB
    static constexpr size_t MAX_SIZE_BYTES =
        __MAX_SHARED_MEM_SIZE_MIB * 1024 * 1024;

    SharedMemory(std::string_view shMemName, size_t requestedSize);
    ~SharedMemory();

    // No default/copy/move construction
    EXPLICIT_DELETE_CONSTRUCTORS(SharedMemory);

    // For accessing shared memory as struct by reinterpretation
    template <typename T>
    [[nodiscard]] T *AsStruct() const {
        T *data{nullptr};

        // Make sure that m_Data is mapped, and check that it fits exactly into
        // type T
        if (m_Data && sizeof(T) == m_DataSize) {
            data = reinterpret_cast<T *>(m_Data);
        }

        return data;
    }

    // For accessing shared memory as std::span of contiguous data
    template <typename T>
    [[nodiscard]] std::span<T> AsSpan() const {
        T *data{nullptr};
        size_t size{0};

        // Make sure that m_Data is mapped
        if (m_Data) {
            data = reinterpret_cast<T *>(m_Data);
            size = m_DataSize;
        }

        return std::span<T>(data, size);
    }

    [[nodiscard]] std::string Name() const { return m_Name; }
    [[nodiscard]] size_t Size() const { return m_DataSize; }
    [[nodiscard]] int ReferenceCount() const;

private:
    // Open a shared memory location using shm_open. Returns false if shared
    // memory does not exist
    bool OpenSharedMemFile(std::string_view name);
    // Close shared memory file without unlinking
    void CloseSharedMemFile() noexcept;

    // Create a shared memory location if it doesn't exist using shm_open
    void AllocSharedMem(std::string_view name);
    // Unlink a shared memory location using shm_unlink
    void FreeSharedMem() noexcept;

    // Map shared memory to our process's virtual memory and link our pointers
    // to the correct locations in the shared memory region
    void MapSharedMem(std::string_view name = {});
    // Unmap shared memory from our process's virtual memory and unlink our
    // pointers
    void UnmapSharedMem() noexcept;

    // Name of shared memory region - used for linking/unlinking
    char *m_Name{nullptr};
    // Reference counter to control shared memory lifetime. Set by
    // `MapSharedMem`, reset by `UnmapSharedMem`.
    int *m_RefCounter{nullptr};
    // Actual underlying data - the stuff we care about. Set by `MapSharedMem`,
    // reset by `UnmapSharedMem`.
    void *m_Data{nullptr};
    // Size in bytes of shared data region
    const size_t m_DataSize;
    // Size in bytes of shared data region plus reference counter
    const size_t m_TotalSize;
    // For storing the FD that describes our shared memory
    int m_FileDes{-1};
    // Semaphore lock for synchronizing linking/unlinking of shared memory
    SemaphoreLock m_SemaphoreLock;
};
