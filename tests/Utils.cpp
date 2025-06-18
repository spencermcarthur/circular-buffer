#include "Utils.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cerrno>

bool CheckSharedMemExists(const char *name) {
    bool exists{true};
    int fileDesc = shm_open(name, O_RDONLY, 0);
    if (fileDesc == -1 && errno == ENOENT) {
        exists = false;
    }
    close(fileDesc);
    return exists;
}

void UnlinkSharedMem(const char *name) { shm_unlink(name); }
