# Task: Circular Buffer Implementation

> Implement a circular memory buffer capable of handling messages of arbitrary length,
with each message being less than 2<sup>16</sup> bytes. The buffer will facilitate inter-process
communication using a shared memory segment and should support asynchronous
read and write operations. The buffer size will be specified at construction.
If the reader process becomes too slow and the data it intends to read gets overwritten,
it must detect this condition, print an error message, and terminate.
The implementation must include tests using Google Test (GTest).

# Outline: Approach
- C++ Standard: 20
- Linux operating system (I'm using WSL 2 running Ubuntu 24.04.1 LTS)
- IPC: shared memory using Linux system calls
    - `shm_open`/`shm_unlink` for requesting shared memory from the kernel
    - `mmap`/`munmap` for mapping shared memory to process virtual memory

# Design

## Components

### `SharedMemory`
RAII class for managing access to and lifetime of shared memory. Maintains an atomic reference count stored at beginning of shared memory location.

At construction, requests access to named shared memory via `shm_open`. If requested shared memory doesn't exists, handles creation via that same syscall. Once access is acquired, maps shared memory to the calling process's virtual memory via `mmap`.

At destruction, unmaps shared memory from calling process's virtual memory via `munmap`. If reference count drops to 0, calls `shm_unlink` to schedule freeing of the shared memory location.

Allows reinterpretation of the shared memory as a struct or a contiguous `span` of data via template methods.

### `CircularBuffer::IWrapper`
Interface class that owns `SharedMemory` objects and wraps around the buffer indices and data. Facilitates the simple implementation of `Reader` and `Writer` instances.

### `CircularBuffer::Reader`
`IWrapper` implementation that handles asynchronous reads from the buffer. Implements method `int Read(span<byte>)`.

### `CircularBuffer::Writer`
`IWrapper` implementation that handles asynchronous writes to the buffer. Implements method `void Write(span<byte>)`.

## Testing

# References
- [shm_open(3) — Linux manual page](https://man7.org/linux/man-pages/man3/shm_open.3.html)
- [mmap(2) — Linux manual page](https://man7.org/linux/man-pages/man2/mmap.2.html)
- [When Nanoseconds Matter: Ultrafast Trading Systems in C++ - David Gross - CppCon 2024](https://www.youtube.com/watch?v=sX2nF1fW7kI)