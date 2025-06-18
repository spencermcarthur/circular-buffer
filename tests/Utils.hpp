#pragma once

#include <cstddef>

bool SharedMemExists(const char *name);
void RequestSharedMem(const char*name, size_t size);
void FreeSharedMem(const char *name);
