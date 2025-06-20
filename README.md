# Circular Buffer

## Details
- Arbitrary-length messages of from 1B to (2<sup>16</sup>-1)B
- Shared memory for IPC between one writer process and multiple reader processes
- Dynamic buffer size up to 50MiB
- Reader overwrite detection

## Requirements
- C++ 20
- Linux kernel (for syscalls)

## Dependencies
- libgtest
- libbenchmark
- libspdlog (bundled)

## Approach
- Shared memory using Linux/POSIX syscalls
    - `shm_open`/`shm_unlink` for requesting shared memory from the kernel
    - `mmap`/`munmap` for mapping shared memory to process virtual memory
    - semaphores for coordinating allocation/deallocation of shared memory between independent processes

## Components

### `SemaphoreLock`
A semaphore lock utility class for coordinating allocation/freeing of shared memory between process.

### `SharedMemory`
An RAII class for managing access to and lifetime of shared memory. Maintains an atomic reference counter on a dedicated cacheline at the beginning of the shared memory location.

At construction, it requests access to the named shared memory via `shm_open`. If the requested shared memory has not yet been allocated, it handles allocation via that same syscall. Once access is acquired, it maps the shared memory to the calling process' virtual memory via `mmap`.

At destruction, it unmaps the shared memory from the calling process' virtual memory via `munmap`. If the reference count of the underling `SharedMemory` drops to 0, it calls `shm_unlink` to free the shared memory.

It uses `SemaphoreLock` for the allocation/free calls to ensure coordination between independent processes.

It allows reinterpretation of the shared memory as a struct (`T* AsStruct<T>()`) or contiguous memory (`std::span<T> AsSpan<T>()`).

### `CircularBuffer::Spec`
A POD structure to convey information about buffer shared memory names and buffer size.

### `CircularBuffer::State`
A POD structure to maintain state information about the buffer in shared memory, include atomic read and write pointers. Owned and managed by `IWrapper` interface and used by `Reader` and `Writer`.

### `CircularBuffer::IWrapper`
An interface class that owns `SharedMemory` objects that manage access to buffer indices and data. It facilitates the simple implementation of `Reader` and `Writer`. It takes a `CircularBuffer::Spec const&` for construction.

### `CircularBuffer::Reader`
`IWrapper` implementation that facilitates reading from the buffer. Implements method `int Read(span<byte>)`.

### `CircularBuffer::Writer`
`IWrapper` implementation that facilitates writing to the buffer. Implements method `void Write(span<byte>)`.

## Testing

### Unit Tests
`SemaphoreLock`
1. Constructor
2. Constructor failure cases
    - Invalid name cases (too long/short)
3. Acquire/release logic
    - Multiple objects accessing the same semaphore

`SharedMemory`
1. Constructor
2. Constructor failure cases
    - Invalid name cases (too long/short)
    - Invalid size requested (too large or 0)
3. Construction of multiple objects
    - Reference counter works
    - Shared memory is freed after last object goes out of scope

## References
- [When Nanoseconds Matter: Ultrafast Trading Systems in C++ - David Gross - CppCon 2024](https://www.youtube.com/watch?v=sX2nF1fW7kI)
- [shm_open(3) — Linux manual page](https://man7.org/linux/man-pages/man3/shm_open.3.html)
- [mmap(2) — Linux manual page](https://man7.org/linux/man-pages/man2/mmap.2.html)