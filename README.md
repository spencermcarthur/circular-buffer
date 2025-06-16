# Task: Circular Buffer Implementation

Implement a circular memory buffer capable of handling messages of arbitrary length,
with each message being less than 2<sup>16</sup> bytes. The buffer will facilitate inter-process
communication using a shared memory segment and should support asynchronous
read and write operations. The buffer size will be specified at construction.
If the reader process becomes too slow and the data it intends to read gets overwritten,
it must detect this condition, print an error message, and terminate.
The implementation must include tests using Google Test (GTest).

# Approach
- C++ Standard: 20
- 