#include "Utils.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <format>
#include <stdexcept>

bool SharedMemExists(const char *name) {
    bool exists{true};
    int fileDesc = shm_open(name, O_RDONLY, 0);
    if (fileDesc == -1 && errno == ENOENT) {
        exists = false;
    }
    close(fileDesc);
    return exists;
}

void RequestSharedMem(const char *name, size_t size) {
    int fileDesc = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0);
    if (fileDesc == -1) {
        return;
    }

    if (ftruncate(fileDesc, size) == -1) {
        throw std::runtime_error(std::format(
            "({}:{}) Failed to allocate memory for shared memory {}", __FILE__,
            __LINE__, name));
    }
}

void FreeSharedMem(const char *name) { shm_unlink(name); }

using namespace CircularBuffer;

BufferT MakeBuffer(const size_t size, char fill) {
    DataT *data = new DataT[size]{};
    if (fill != '\0') {
        std::memset(data, fill, size);
    }
    return BufferT{data, size};
}
