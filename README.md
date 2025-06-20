# Circular Buffer

## Features
✅ Arbitrary-length messages from 1 to 65535 bytes \
✅ Shared memory for IPC between one writer process and multiple reader processes \
✅ Dynamic buffer size up to 50 MiB[^1] \
✅ Reader overwrite detection

## Requirements
- C++ 20
- Linux kernel (for syscalls)

## External Build Dependencies
- libgtest (unit tests only)
- libbenchmark (benchmarks only)

## Building
```
git clone git@github.com:spencermcarthur/circular-buffer.git
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug|Release ..
cmake --build .
```
Optional targets: `UnitTests`, `Benchmarks`, `Apps`

## Approach
- Size-prefixed buffer schema
    - Message size is written as an unsigned 32-bit integer in front of the message data
- Shared memory using Linux/POSIX syscalls
    - `shm_open`/`shm_unlink` for allocating/freeing shared memory from the kernel
    - `mmap`/`munmap` for mapping/unmapping shared memory to process virtual memory
    - semaphores for coordinating allocation/deallocation of shared memory between independent processes

## Components

### `SemaphoreLock`
A semaphore lock utility class for coordinating allocation/freeing of shared memory between process.

### `SharedMemory`
An RAII class for managing access to and lifetime of shared memory. Maintains an atomic reference counter on a dedicated cacheline at the beginning of the shared memory location.

At construction, it requests access to the named shared memory via `shm_open`. If the requested shared memory has not yet been allocated, it handles allocation via that same syscall. Once access is acquired, it maps the shared memory to the calling process' virtual memory via `mmap`.

At destruction, it unmaps the shared memory from the calling process' virtual memory via `munmap`. If the reference count of the underling `SharedMemory` drops to 0, it calls `shm_unlink` to free the shared memory.

It uses `SemaphoreLock` for the allocation/free calls to ensure coordination between independent processes.

Public template methods allow reinterpretation of the shared memory as a struct (`AsStruct()`) or a contiguous range of data (`AsSpan()`).

### `CircularBuffer::Spec`
A POD structure to convey information about buffer shared memory names and buffer size.

### `CircularBuffer::State`
A POD structure to maintain state information about the buffer in shared memory, include atomic read and write pointers. Owned and managed by `IWrapper` interface and used for read and write operations.

### `CircularBuffer::IWrapper`
An interface class that owns `SharedMemory` objects that manage access to buffer indices and data. It facilitates the simple implementation of `Reader` and `Writer`. It takes a `CircularBuffer::Spec const&` for construction.

### `CircularBuffer::Reader`
`IWrapper` implementation that facilitates reading from the buffer. Implements public `Read()` methods.

### `CircularBuffer::Writer`
`IWrapper` implementation that facilitates writing to the buffer. Implements public `Write()` methods.

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

### Integration Tests
`ReaderApp` and `WriterApp`
- Run writer and reader in separate processes to demonstrate IPC integration
- Reader can be run in "slow" mode to demonstrate overwrite detection: `./ReaderApp slow`

## References
- [When Nanoseconds Matter: Ultrafast Trading Systems in C++ - David Gross - CppCon 2024](https://www.youtube.com/watch?v=sX2nF1fW7kI)
- [shm_overview(7) — Linux manual page](https://man7.org/linux/man-pages/man7/shm_overview.7.html)
- [sem_overview(7) — Linux manual page](https://man7.org/linux/man-pages/man7/sem_overview.7.html)
- [ccpreference](https://cppreference.com/)

## Notes
[^1]: Somewhat arbitrary, but benchmarks show that writing large messages into a large buffer is less performant than large messages into a small buffer, probably due to cache contention.
