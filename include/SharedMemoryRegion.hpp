#pragma once

#include <cstddef>
#include <linux/limits.h>
#include <span>

// Class for managing Linux shared memory
class SharedMemoryRegion {
  // 500 MiB
  static constexpr size_t MAX_SHMMEM_SIZE = 500 * 1024 * 1024;

public:
  SharedMemoryRegion(const char *memoryRegionName, size_t requestedSize,
                     bool isReadOnly = false);
  ~SharedMemoryRegion();

  // No default/copy/move construction
  SharedMemoryRegion() = delete;
  SharedMemoryRegion(const SharedMemoryRegion &) = delete;
  SharedMemoryRegion &operator=(const SharedMemoryRegion &) = delete;
  SharedMemoryRegion(SharedMemoryRegion &&) = delete;
  SharedMemoryRegion &operator=(SharedMemoryRegion &&) = delete;

  // For accessing shared memory as a shared data structure by reinterpretation
  template <typename T> T *Data() const;

  // For accessing shared memory as contiguous data, e.g. std::span<std::byte>
  template <typename T> std::span<T> Data() const;

  const char *Name() const { return m_Name; }
  size_t Size() const { return m_DataSize; }
  bool IsReadOnly() const { return m_IsReadOnly; }
  int ReferenceCount() const;

private:
  // Open a shared memory location using shm_open. Returns false if shared
  // memory does not exist
  bool OpenSharedMem();
  // Close shared memory file without unlinking
  void CloseSharedMem() noexcept;
  // Create a shared memory location if it doesn't exist using shm_open
  void LinkSharedMem();
  // Unlink a shared memory location using shm_unlink
  void UnlinkSharedMem() noexcept;

  // Map shared memory to our process's virtual memory and link our pointers to
  // the correct locations in the shared memory region
  void MapSharedMem();
  // Unmap shared memory from our process's virtual memory and unlink our
  // pointers
  void UnmapSharedMem() noexcept;

  // Name of shared memory region - used for linking/unlinking
  char *m_Name{nullptr};
  // Reference counter to control shared memory lifetime
  int *m_RefCounter{nullptr};
  // Actual underlying data - the stuff we care about
  void *m_Data{nullptr};
  // Size in bytes of shared data region
  const size_t m_DataSize;
  // Size in bytes of shared data region plus reference counter
  const size_t m_TotalSize;
  // For storing the FD that describes our shared memory
  int m_FileDes{-1};
  // If the mapping should be read-only
  const bool m_IsReadOnly;
};
