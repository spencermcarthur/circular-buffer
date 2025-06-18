#include "Utils.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cerrno>

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

    ftruncate(fileDesc, size);
}

void FreeSharedMem(const char *name) { shm_unlink(name); }
