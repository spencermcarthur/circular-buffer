#include "SharedMemory.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string_view>

#include "Macros.hpp"
#include "SemaphoreLock.hpp"
#include "Utils.hpp"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"

SharedMemory::SharedMemory(const std::string_view name,
                           const size_t requestedSize)
    : m_DataSize(requestedSize),
      m_TotalSize(requestedSize + DATA_OFFSET_BYTES),
      m_SemLock(name) {
    SetupSpdlog();

    const size_t nameLen = name.length();

    // Validate args
    if (nameLen < 1 || nameLen > MAX_NAME_LEN) {
        CONSTEXPR_SV fmt =
            "({}:{}) Memory region name {} of length {} is invalid: length "
            "must be 0 < len <= {}";
        SPDLOG_ERROR(fmt.substr(8), name, nameLen, MAX_NAME_LEN);
        throw std::length_error(
            std::format(fmt, __FILE__, __LINE__, name, nameLen, MAX_NAME_LEN));
    }
    if (requestedSize < 1 || requestedSize > MAX_SIZE_BYTES) {
        CONSTEXPR_SV fmt =
            "({}:{}) Requested memory region of size {} B is "
            "invalid: size must be between 1 and {} bytes";
        SPDLOG_ERROR(fmt.substr(8), requestedSize, MAX_SIZE_BYTES);
        throw std::domain_error(std::format(fmt, __FILE__, __LINE__,
                                            requestedSize, MAX_SIZE_BYTES));
    }

    // If we can't open shared memory at m_Name
    if (!OpenSharedMemFile(name)) {
        // Try to create it
        AllocSharedMem(name);

        // If we succeeded in creating it (i.e. didn't crash) but still can't
        // open it, then fail
        if (!OpenSharedMemFile(name)) {
            const int err = errno;
            CONSTEXPR_SV fmt =
                "({}:{}) Failed to open shared memory for {}: {}";
            SPDLOG_ERROR(fmt.substr(8), name, strerror(err));
            throw std::runtime_error(
                std::format(fmt, __FILE__, __LINE__, name, strerror(err)));
        }
    }

    // Map the data to our virtual memory
    MapSharedMem(name);

    // Copy name
    m_Name = new char[nameLen + 1]{};
    std::strncpy(m_Name, name.data(), nameLen);

    // Increment ref counter
    const std::atomic_ref<int> refCounter(*m_RefCounter);
    refCounter.fetch_add(1, std::memory_order_release);
}

SharedMemory::~SharedMemory() {
    // Check before dereferencing
    if (m_RefCounter != nullptr) {
        const std::atomic_ref<int> refCounter(*m_RefCounter);

        // Decrement ref counter, and capture value before CAS operation
        const int refCount =
            refCounter.fetch_sub(1, std::memory_order_release) - 1;

        // Unmap from process virt mem
        UnmapSharedMem();

        // If ref count is 0, schedule free
        if (refCount == 0) {
            FreeSharedMem();
        }
    }

    // Close file handle
    CloseSharedMemFile();

    delete[] m_Name;
}

int SharedMemory::ReferenceCount() const {
    int refCount{-1};
    if (m_RefCounter != nullptr) {
        refCount =
            std::atomic_ref<int>(*m_RefCounter).load(std::memory_order_acquire);
    }
    return refCount;
}

bool SharedMemory::OpenSharedMemFile(std::string_view name) {
    // Try to open shared memory file
    const int fileDesc = shm_open(name.data(), O_RDWR, S_IRUSR + S_IWUSR);
    if (fileDesc == -1) {
        // Failed

#ifdef DEBUG
        const int err = errno;
        SPDLOG_DEBUG("Failed to open shared memory file {}: {}", name,
                     strerror(err));
#endif

        return false;
    }

    // Get file info
    struct stat buf;
    if (fstat(fileDesc, &buf) == -1) {
        // Failed
        const int err = errno;
        CONSTEXPR_SV fmt = "({}:{}) fstat failed for shared memory file {}: {}";
        SPDLOG_ERROR(fmt.substr(8), name, strerror(err));
        throw std::runtime_error(
            std::format(fmt, __FILE__, __LINE__, name, strerror(err)));
    }

    // Check that existing shared memory's size is what's expected
    if (static_cast<size_t>(buf.st_size) != m_TotalSize) {
        CONSTEXPR_SV fmt =
            "({}:{}) Requested shared memory size {} does not match existing "
            "shared memory size {} for {}";
        SPDLOG_ERROR(fmt.substr(8), buf.st_size, m_DataSize, name);
        throw std::runtime_error(std::format(fmt, __FILE__, __LINE__,
                                             buf.st_size, m_DataSize, name));
    }

    SPDLOG_DEBUG("Opened shared memory {}", name);

    m_FileDes = fileDesc;
    return true;
}

void SharedMemory::CloseSharedMemFile() noexcept {
    if (close(m_FileDes) == -1) {
        // Failed
        const int err = errno;
        SPDLOG_ERROR("Failed to close shared memory file descriptor for {}: {}",
                     m_Name, strerror(err));
        return;
    }

    SPDLOG_DEBUG("Closed shared memory file descriptor for {}", m_Name);

    m_FileDes = -1;
}

void SharedMemory::AllocSharedMem(std::string_view name) {
    // Try to lock semaphore
    if (!m_SemLock.Acquire()) {
        return;
    }

    // Create new shared memory in system
    const int fileDesc =
        shm_open(name.data(), O_RDWR | O_CREAT | O_EXCL, S_IRUSR + S_IWUSR);
    if (fileDesc == -1) {
        // Failed
        const int err = errno;
        CONSTEXPR_SV fmt =
            "({}:{}) Failed to create shared memory entry for name {}: {}";
        SPDLOG_ERROR(fmt.substr(7), name, strerror(err));
        throw std::runtime_error(
            std::format(fmt, __FILE__, __LINE__, name, strerror(err)));
    }

    // Allocate m_Size bytes
    if (ftruncate(fileDesc, m_TotalSize) == -1) {
        // Failed
        const int err = errno;
        CONSTEXPR_SV fmt =
            "({}:{}) failed to allocate shared memory for name {}: {}";
        SPDLOG_ERROR(fmt.substr(8), name, strerror(err));
        throw std::runtime_error(
            std::format(fmt, __FILE__, __LINE__, name, strerror(err)));
    }

    SPDLOG_DEBUG("Allocated shared memory {} of size {}", name, m_DataSize);

    m_SemLock.Release();
}

void SharedMemory::FreeSharedMem() noexcept {
    // Try to lock semaphore before unlinking
    if (!m_SemLock.Acquire()) {
        return;
    }

    if (shm_unlink(m_Name) == -1) {
        // Failed
        const int err = errno;
        SPDLOG_ERROR("Failed to free shared memory {}: {}", m_Name,
                     strerror(err));
        return;
    }

    SPDLOG_DEBUG("Freed shared memory {}", m_Name);

    m_SemLock.Release();
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void SharedMemory::MapSharedMem(std::string_view name) {
#pragma GCC diagnostic pop
    // Map shared memory to our process's virtual memory
    void *data = mmap(nullptr, m_TotalSize, PROT_READ | PROT_WRITE, MAP_SHARED,
                      m_FileDes, 0);
    if (data == MAP_FAILED) {
        // Failed to map
        const int err = errno;
        CONSTEXPR_SV fmt = "({}:{}) Failed to map shared memory: {}";
        SPDLOG_ERROR(fmt.substr(8), strerror(err));
        throw std::runtime_error(
            std::format(fmt, __FILE__, __LINE__, strerror(err)));
    }

    SPDLOG_DEBUG("Mapped shared memory {}", name);

    m_RefCounter = reinterpret_cast<int *>(data);
    m_Data = reinterpret_cast<void *>(m_RefCounter + DATA_OFFSET_BYTES);
}

void SharedMemory::UnmapSharedMem() noexcept {
    if (munmap(reinterpret_cast<void *>(m_RefCounter), m_TotalSize) == -1) {
        // Failed to unmap
        const int err = errno;
        SPDLOG_ERROR("Failed to unmap shared memory: {}", strerror(err));
        return;
    }

    SPDLOG_DEBUG("Unmapped shared memory {}", m_Name);

    m_RefCounter = nullptr;
    m_Data = nullptr;
}
