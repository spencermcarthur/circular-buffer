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
To build libcircularbuffer:
```
git clone git@github.com:spencermcarthur/circular-buffer.git
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug|Release ..
cmake --build .
```
Optional targets: UnitTests, Benchmarks, Apps

## Design
- Size-prefixed buffer scheme
    - Message size is written as a prefix to the message data
    - Reader first reads the message size and then the corresponding number of bytes after
- Shared memory accessed using Linux/POSIX syscalls
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
A POD structure to maintain global, atomic state information about the buffer in shared memory, namely the read index, write index, and sequence number (i.e. the total number of bytes that have been written to the buffer). A copy is owned and managed by the `IWrapper` interface and used for read and write operations.

### `CircularBuffer::Reader`
`IWrapper` implementation that facilitates reading from the buffer. Implements public `Read()` methods.

### `CircularBuffer::Writer`
`IWrapper` implementation that facilitates writing to the buffer. Implements public `Write()` methods.

### `CircularBuffer::IWrapper`
An interface class that owns `SharedMemory` objects that manage access to buffer state and data. It facilitates the simple implementation of `Reader` and `Writer`. It takes a `CircularBuffer::Spec const&` for construction.

In addition to buffer state and data, it maintains two protected member `uint64_t` variables.

The first (`m_LocalIndex`) maintains a (non-atomic) location in the buffer owned by derived classes that implement the `IWrapper` interface. For the writer, this variable maintains the next position to be written to, and is used to pre-compute the length of a write operation and next location for the global write and read indices. This allows an atomic store to the global indices rather than a more expensive atomic CAS. For the reader, this varialbe maintains its next read index. It compares this local value to the global read index, and if the read index has advanced, that is a signal from the writer that its write operation is complete and the reader may read the next message in the buffer.

The second variable (`m_LocalSeqNum`) maintains a local count of total bytes written or read by the writer or reader, respectively. This is the mechanism that the reader uses to check if it has been overwritten. Specifically, the reader atomically loads the global sequence number and performs the comparison `global - local > buffer_size`. If this comparison resolves to true, then the reader knows it has been overwritten because the writer has "lapped" it.


## Algorithm

### Writer
The writer algorithm is somewhat simpler than the reader because it does not perform overwrite checks. The general sequence is as follows:

1. Atomically move the global write index ahead by `n` bytes, where `n = header_size + message_size`
2. Write the header (size of the message), followed by the message itself
3. Atomically increment the global sequence number by `n`
4. Atomically move the global read index ahead by `n`, signaling to the readers that the write is complete

The writer handles wraparound by doing a partial write at the end of the buffer and writing the remaining data at the beginning of the buffer. However, it may be preferable to set a flag and then write the whole sequence at the beginning of the buffer to avoid multiple writes. For example, the writer could write a header indicating a message size of zero, which would signal the readers to go to the beginning of the buffer for the next message. However, this does not enable the writer to utilize the entire buffer, and could increase the chances of an overwrite.


### Reader
The reader algorithm is essentially analagous to the writer algorithm with a few minor differences. The general sequence is:

1. Check the local index against the global read index to see if there is any data to read
2. Read the message size from the buffer
3. Read the message itself from the buffer
4. Move the local index ahead by `n` bytes
5. Return the message size to the caller

The reader performs the same wraparound checks as the writer and reads the data in the same way that the writer writes it. The major difference is that the reader is responsible for its own overwrite detection. It manages this by checking the local sequence number against the global sequence number. If `global - local > buffer_size`, it indicates that the writer has "lapped" the reader. The reader then logs an error and terminates. It performs this check before and after the read.


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
