#pragma once

#include <cstddef>

#include "circularbuffer/Aliases.hpp"

bool SharedMemExists(const char *name);
void RequestSharedMem(const char *name, size_t size);
void FreeSharedMem(const char *name);

CircularBuffer::BufferT MakeBuffer(size_t size, char fill = '\0');
