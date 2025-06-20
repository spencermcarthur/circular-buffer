# Circular Buffer

## Features
✅ Arbitrary-length messages from 1 to 65535 bytes \
✅ Shared memory for IPC between one writer process and multiple reader processes \
✅ Asynchronous read/write \
✅ Dynamic buffer size defined at runtime[^1] \
✅ Reader overwrite detection

## Requirements
- C++20 STL
- GCC or Clang compilers with C++20 support
- Linux
- CMake

## External Dependencies
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

### Optional Targets
- `UnitTests`
- `Benchmarks`
- `Apps`

### Optional CMake Command Line Definitions
- `MAX_SHARED_MEM_SIZE_MIB`: controls the maximum allowed size of a shared memory region, which results in a limitation on buffer size. Default: 50 MiB
- `MAX_MESSAGE_SIZE_BYTES`: controls the maximum allowed size of a message. Default: 65535 B
- `GETCONF_CACHELINE_SIZE_VAR`: controls the variable used to retrieve the CPU cacheline size at compile time. CMake calls `getconf` with this argument and defines it as a macro, which is then used for alignment of certain data structures to prevent false sharing of atomic data. Default: `LEVEL1_DCACHE_LINESIZE`

## Design

### Outline
- Size-prefixed buffer scheme
    - Message size header is written as a prefix to the message data
    - Reader first reads the message size and then the corresponding number of bytes after to retrieve the message
- Shared memory accessed using Linux/POSIX syscalls
    - `shm_open`/`shm_unlink` for allocating/freeing shared memory from the kernel
    - `mmap`/`munmap` for mapping/unmapping shared memory to process virtual memory
    - semaphores for coordinating allocation/deallocation of shared memory between independent processes
- Atomic operations for coordinating buffer state
    - Buffer state (read index, write index, and sequence number) is accessed atomically
    - Writer writes state atomically, reader reads state atomically, no direct coordination/synchronization required between processes at runtime
    - Writer first moves the write index to "reserve" space in the buffer, then writes the data, and finally moves the read index[^2] and updates the sequence number to reflect the number of bytes written during an operation
    - Reader reads up to the read index and stops if it reaches it
- Overwrite detection
    - If the reader lags behind the writer by more than the size of the buffer, it logs an error and returns an error code

### Components

#### `SemaphoreLock`
A semaphore lock utility class for coordinating allocation/freeing of shared memory between process. Also used by the writer to ensure a singleton writer per buffer on the kernel.

#### `SharedMemory`
An RAII class for managing access to and lifetime of shared memory. Maintains an atomic reference counter on a dedicated cacheline at the beginning of the shared memory location.

At construction, it requests access to the named shared memory via `shm_open`. If the requested shared memory has not yet been allocated, it handles allocation via that same syscall. Once access is acquired, it maps the shared memory to the calling process' virtual memory via `mmap`.

At destruction, it unmaps the shared memory from the calling process' virtual memory via `munmap`. If the reference count of the underling `SharedMemory` drops to 0, it calls `shm_unlink` to free the shared memory.

It uses `SemaphoreLock` for the allocation/free calls to ensure coordination between independent processes.

Public template methods allow reinterpretation of the shared memory as a simple data structure (`AsStruct()`) or a contiguous range of data (`AsSpan()`).

#### `CircularBuffer::Spec`
A [POD structure](https://en.wikipedia.org/wiki/Passive_data_structure) to convey information about buffer shared memory names and buffer size.

#### `CircularBuffer::State`
A POD structure to maintain global, atomic state information about the buffer in shared memory, namely the read index, write index, and sequence number (i.e. the total number of bytes that have been written to the buffer). A copy is owned and managed by the `IWrapper` interface and used for read and write operations. Writer and reader copies reference the same shared memory location.

#### `CircularBuffer::Reader`
An simple class that facilitates reading from the buffer. Implements `IWrapper` interface as well as public `Read()` methods.

#### `CircularBuffer::Writer`
An simple class that facilitates writing to the buffer. Implements `IWrapper` interface as well as public `Write()` methods.

#### `CircularBuffer::IWrapper`
An interface class that owns `SharedMemory` objects that manage access to buffer state and data. It facilitates the simple implementation of `Reader` and `Writer`. It takes a `CircularBuffer::Spec const&` for construction.

In addition to buffer state and data, it maintains two protected member `uint64_t` variables:

The first (`m_LocalIndex`) maintains a (non-atomic) local index in the buffer, and is managed by the `Reader` and `Writer` instances independently. For the writer, this variable maintains the next position to be written to, and is used to pre-compute the length of a write operation and next location for the global write and read indices. This allows an atomic store to the global indices rather than a more expensive atomic CAS. For the reader, this varialbe maintains its next read index. It compares this local value to the global read index, and if the read index has advanced, that is a signal from the writer that its write operation is complete and the reader may read the next message in the buffer.

The second variable (`m_LocalSeqNum`) maintains a local count of total bytes written or read by the writer or reader, respectively. This is the mechanism that the reader uses to check if it has been overwritten. Specifically, the reader atomically loads the global sequence number and performs the comparison `global - local > buffer_size`. If this comparison resolves to true, then the reader knows it has been overwritten because the writer has "lapped" it.


### Algorithm

#### Writer
The writer algorithm is simple in that it is agnostic to any other code that is accessing the buffer. It maintains sole ownership of the buffer, and acts as if exists in isolation. This ensures fast write times due to the lack of need for coordinating with readers.

The general sequence is as follows:

1. Atomically move the global write index ahead by `n` bytes, where `n = header_size + message_size`
2. Write the header (size of the message), followed by the message itself
3. Atomically increment the global sequence number by `n`
4. Atomically move the global read index ahead by `n`, which signals to the readers that the write is complete and they may begin reading up to that index

The writer handles wraparound by doing a partial write at the end of the buffer and writing the remaining data at the beginning of the buffer. It may be preferable to set a flag and then write the whole sequence at the beginning of the buffer to avoid multiple writes. For example, the writer could write a header indicating a message size of zero, which would signal the readers to go to the beginning of the buffer for the next message. However, this does not enable the writer to utilize the entire buffer, and could increase the likelihood of an overwrite on a slow reader.

#### Reader
The reader algorithm is essentially analagous to the writer algorithm with a few minor differences. Because the reader is responsible for detecting its own overwrite, it necessarily needs to coordinate with the writer in a few ways.

The general sequence is as follows:

1. Check the local index against the global read index to see if there is any data to read
2. Check if it has been overwritten by the writer
3. Read the message size from the buffer
4. Read the message itself from the buffer
5. Move the local index ahead by `n` bytes, where `n = header_size + message_size`
6. Check for overwrite again
7. Return the message size to the caller

The reader performs the same wraparound checks as the writer, and reads the data in a way that is analogous to how the writer writes it. The reader performs overwrite detection by checking its local sequence number against the global atomic sequence number. If `global - local > buffer_size`, it indicates that the writer has "lapped" the reader. The reader then logs an error and terminates. It performs this check before and after the read, which is important for ensuring the integrity of the data. The check before reading ensures that the reader is reading accurate data, and helps avoid erroneous or out-of-bounds reads caused by a corrupted header, e.g. if the message size read from the buffer is invalid. The check after reading ensures that no overwrite occurred during the read, which would lead to a race condition.

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
`ReaderApp`, `WriterApp`, `ReaderWriterApp`
- Run writer and reader in separate processes to demonstrate IPC integration
- Reader can be run in "slow" mode to demonstrate overwrite detection: `./ReaderApp slow`
- Writer can be run if "fase" mode to speed up demonstration of overwrite: `./WriterApp fast`
- Running in Debug configuration displays log messages that are not present in Release, and also performs additional sanity checks in the `Reader` and `Writer` code to ensure the algorithms are operating as expected.

## References
1. [When Nanoseconds Matter: Ultrafast Trading Systems in C++ - David Gross - CppCon 2024 (YouTube)](https://www.youtube.com/watch?v=sX2nF1fW7kI)
2. [When Nanoseconds Matter: Ultrafast Trading Systems in C++ - David Gross - CppCon 2024 (PDF)](https://github.com/CppCon/CppCon2024/blob/main/Presentations/When_Nanoseconds_Matter.pdf)
3. [shm_overview(7) — Linux manual page](https://man7.org/linux/man-pages/man7/shm_overview.7.html)
4. [sem_overview(7) — Linux manual page](https://man7.org/linux/man-pages/man7/sem_overview.7.html)
5. [ccpreference](https://cppreference.com/)

[^1]: Up to 50 MiB. This is a somewhat arbitrary limitation and can be changed at compile time via command-line input, but benchmarks show that writing to a very large buffer is less performant than a small/medium buffer, probably due to cache contention.

[^2]: See slides 84-86 of ref. 2 for a visualization.