#include "Utils.hpp"

#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

bool CheckSharedMemExists(const char *name) {
  bool exists{true};
  int fd = shm_open(name, O_RDONLY, 0);
  if (fd == -1 && errno == ENOENT) {
    exists = false;
  }
  close(fd);
  return exists;
}

void UnlinkSharedMem(const char *name) { shm_unlink(name); }
