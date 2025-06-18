#include "SharedMemoryRegion.hpp"

#include <atomic>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <format>
#include <iostream>
#include <linux/limits.h>
#include <span>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

SharedMemoryRegion::SharedMemoryRegion(const char *name, size_t requestedSize,
                                       bool readOnly)
    : m_DataSize(requestedSize), m_TotalSize(requestedSize + sizeof(int)),
      m_IsReadOnly(readOnly) {
  // Validate args
  const size_t nameLen = std::strlen(name);
  if (nameLen > NAME_MAX) {
    throw std::length_error("Name of memory region must be <= 255");
  } else if (requestedSize < 1 || requestedSize > MAX_SHMMEM_SIZE) {
    throw std::domain_error(
        "requestedSize must be between 1B and 500MiB (inclusive)");
  }

  // Copy name
  std::strncpy(m_Name, name, nameLen);

  // If we can't open shared memory at m_Name
  if (!OpenSharedMem()) {
    // Try to create it
    LinkSharedMem();

    // If we succeeded (didn't crash) in creating it but still can't open it,
    // then fail
    if (!OpenSharedMem()) {
      const int err = errno;
      throw std::runtime_error(
          std::format("({}:{}) Failed to open shared memory for {}: {}",
                      __FILE__, __LINE__, m_Name, strerror(err)));
    }
  }

  // Map the data to our virtual memory
  MapSharedMem();

  // Update ref counter
  std::atomic_ref<int>(*m_RefCounter).fetch_add(1, std::memory_order_release);
}

SharedMemoryRegion::~SharedMemoryRegion() {
  // Check before dereferencing
  if (m_RefCounter != nullptr) {
    // Ref count before atomic CAS operation
    const int prevRefCount = std::atomic_ref<int>(*m_RefCounter)
                                 .fetch_sub(1, std::memory_order_release);

    UnmapSharedMem();

    if (prevRefCount == 1) {
      UnlinkSharedMem();
    }

    CloseSharedMem();
  }
}

template <typename T> T *SharedMemoryRegion::Data() const {
  T *data{nullptr};

  // Make sure that m_Data hasn't been unmapped, and check that it fits exactly
  // into type T
  if (m_Data && sizeof(T) == m_DataSize) {
    data = reinterpret_cast<T *>(m_Data);
  }

  return data;
}

template <typename T> std::span<T> SharedMemoryRegion::Data() const {
  T *data{nullptr};
  size_t size{0};

  // Make sure that m_Data hasn't been unmapped
  if (m_Data) {
    data = reinterpret_cast<T *>(m_Data);
    size = m_DataSize;
  }

  return std::span<T>(data, size);
}

std::pair<int, mode_t> GetArgs(bool isReadOnly) {
  int flags;
  mode_t mode;
  if (isReadOnly) {
    flags = O_RDONLY;
    mode = S_IRUSR;
  } else {
    flags = O_RDWR;
    mode = S_IRUSR + S_IWUSR;
  }
  return {flags, mode};
}

int SharedMemoryRegion::ReferenceCount() const {
  int refCount{-1};
  if (m_RefCounter) {
    refCount =
        std::atomic_ref<int>(*m_RefCounter).load(std::memory_order_acquire);
  }
  return refCount;
}

bool SharedMemoryRegion::OpenSharedMem() {
  const auto [flags, mode] = GetArgs(m_IsReadOnly);

  // Try to open shared memory file
  int fd = shm_open(m_Name, flags, mode);
  if (fd == -1) {
    // Failed
    return false;
  }

  // Get file info
  struct stat buf;
  if (fstat(fd, &buf) == -1) {
    // Failed
    const int err = errno;
    throw std::runtime_error(std::format("({}:{}) fstat failed for {}: {}",
                                         __FILE__, __LINE__, m_Name,
                                         strerror(err)));
  }

  // Check that existing shared memory's size is what's expected
  if ((size_t)buf.st_size != m_TotalSize) {
    throw std::runtime_error(
        std::format("({}:{}) requested shared memory size {} does not match "
                    "existing shared memory size {} for name {}",
                    __FILE__, __LINE__, buf.st_size, m_DataSize, m_Name));
  }

  m_FileDes = fd;

  return true;
}

void SharedMemoryRegion::CloseSharedMem() noexcept {
  if (close(m_FileDes) == -1) {
    // Failed
    const int err = errno;
    std::cerr << std::format(
        "({}:{}) Failed to close shared memory file descriptor: {}", __FILE__,
        __LINE__, strerror(err));
  }

  m_FileDes = -1;
}

void SharedMemoryRegion::LinkSharedMem() {
  // Create new shared memory in system
  int fd = shm_open(m_Name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR + S_IWUSR);
  if (fd == -1) {
    // Failed
    const int err = errno;
    throw std::runtime_error(std::format(
        "({}:{}) failed to create shared memory entry for name {}: {}",
        __FILE__, __LINE__, m_Name, strerror(err)));
  }

  // Allocate m_Size bytes
  if (ftruncate(fd, m_TotalSize) == -1) {
    // Failed
    const int err = errno;
    throw std::runtime_error(
        std::format("({}:{}) failed to allocate shared memory for name {}: {}",
                    __FILE__, __LINE__, m_Name, strerror(err)));
  }
}

void SharedMemoryRegion::UnlinkSharedMem() noexcept {
  if (shm_unlink(m_Name) == -1) {
    // Failed
    const int err = errno;
    std::cerr << std::format("({}:{}) Failed to unlink {}: {}", __FILE__,
                             __LINE__, m_Name, strerror(err))
              << std::endl;
  }
}

void SharedMemoryRegion::MapSharedMem() {
  // Map shared memory to our process's virtual memory
  const int prot = m_IsReadOnly ? PROT_READ : PROT_READ | PROT_WRITE;
  void *data = mmap(NULL, m_TotalSize, prot, MAP_SHARED, m_FileDes, 0);
  if (data == MAP_FAILED) {
    // Failed to map
    const int err = errno;
    throw std::runtime_error(std::format("({}:{}) Failed to map: {}", __FILE__,
                                         __LINE__, strerror(err)));
  }

  m_RefCounter = reinterpret_cast<int *>(data);
  m_Data = reinterpret_cast<void *>(m_RefCounter + sizeof(int));
}

void SharedMemoryRegion::UnmapSharedMem() noexcept {
  if (munmap(reinterpret_cast<void *>(m_RefCounter), m_TotalSize) == -1) {
    // Failed to unmap
    const int err = errno;
    std::cerr << std::format("({}:{}) Failed to unmap: {}", __FILE__, __LINE__,
                             strerror(err))
              << std::endl;
  }

  m_RefCounter = nullptr;
  m_Data = nullptr;
}
