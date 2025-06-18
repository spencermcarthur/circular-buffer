#include "SharedMemory.hpp"
#include "SemLock.hpp"

#include <atomic>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <format>
#include <iostream>
#include <linux/limits.h>
#include <semaphore.h>
#include <stdexcept>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

SharedMemory::SharedMemory(std::string_view name, size_t requestedSize)
    : m_DataSize(requestedSize), m_TotalSize(requestedSize + DATA_OFFSET_BYTES),
      m_SemLock(name) {
  const size_t nameLen = name.length();

  // Validate args
  if (nameLen < 1 || nameLen > NAME_MAX) {
    throw std::length_error(
        "Length of memoryRegionName must satisfy 0 < len <= 255");
  } else if (requestedSize < 1 || requestedSize > MAX_SHARED_MEM_SIZE_BYTES) {
    throw std::domain_error(
        "requestedSize must be between 1B and 500MiB (inclusive)");
  }

  // Copy name
  m_Name = new char[nameLen + 1]{};
  std::strncpy(m_Name, name.data(), nameLen);

  // If we can't open shared memory at m_Name
  if (!OpenSharedMem()) {
    // Try to create it
    LinkSharedMem();

    // If we succeeded in creating it (i.e. didn't crash) but still can't open
    // it, then fail
    if (!OpenSharedMem()) {
      const int err = errno;
      throw std::runtime_error(
          std::format("({}:{}) Failed to open shared memory for {}: {}",
                      __FILE__, __LINE__, m_Name, strerror(err)));
    }
  }

  // Map the data to our virtual memory
  MapSharedMem();

  // Increment ref counter
  std::atomic_ref<int> refCount(*m_RefCounter);
  refCount.fetch_add(1, std::memory_order_release);
}

SharedMemory::~SharedMemory() {
  // Check before dereferencing
  if (m_RefCounter != nullptr) {
    std::atomic_ref<int> refCounter(*m_RefCounter);

    // Decrement ref counter, and capture value before CAS operation
    const int refCount = refCounter.fetch_sub(1, std::memory_order_release) - 1;

    // Unmap from process virt mem
    UnmapSharedMem();

    // If ref count is 0, schedule free
    if (refCount == 0) {
      UnlinkSharedMem();
    }
  }

  // Close file handle
  CloseSharedMem();

  delete[] m_Name;
}

int SharedMemory::ReferenceCount() const {
  int refCount{-1};
  if (m_RefCounter) {
    refCount =
        std::atomic_ref<int>(*m_RefCounter).load(std::memory_order_acquire);
  }
  return refCount;
}

bool SharedMemory::OpenSharedMem() {
  // Try to open shared memory file
  int fd = shm_open(m_Name, O_RDWR, S_IRUSR + S_IWUSR);
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

void SharedMemory::CloseSharedMem() noexcept {
  if (close(m_FileDes) == -1) {
    // Failed
    const int err = errno;
    std::cerr << std::format(
        "({}:{}) Failed to close shared memory file descriptor: {}", __FILE__,
        __LINE__, strerror(err));
  }

  m_FileDes = -1;
}

void SharedMemory::LinkSharedMem() {
  // Try to lock semaphore
  if (!m_SemLock.Acquire())
    return;

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

  m_SemLock.Release();
}

void SharedMemory::UnlinkSharedMem() noexcept {
  // Try to lock semaphore before unlinking
  if (!m_SemLock.Acquire())
    return;

  if (shm_unlink(m_Name) == -1) {
    // Failed
    const int err = errno;
    std::cerr << std::format("({}:{}) Failed to unlink {}: {}", __FILE__,
                             __LINE__, m_Name, strerror(err))
              << std::endl;
  }

  m_SemLock.Release();
}

void SharedMemory::MapSharedMem() {
  // Map shared memory to our process's virtual memory
  void *data =
      mmap(NULL, m_TotalSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_FileDes, 0);
  if (data == MAP_FAILED) {
    // Failed to map
    const int err = errno;
    throw std::runtime_error(std::format("({}:{}) Failed to map: {}", __FILE__,
                                         __LINE__, strerror(err)));
  }

  m_RefCounter = reinterpret_cast<int *>(data);
  m_Data = reinterpret_cast<void *>(m_RefCounter + DATA_OFFSET_BYTES);
}

void SharedMemory::UnmapSharedMem() noexcept {
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
