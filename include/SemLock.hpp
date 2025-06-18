#pragma once

#include "Defines.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <format>
#include <iostream>
#include <linux/limits.h>
#include <semaphore.h>
#include <stdexcept>
#include <string_view>

// Semaphore lock class
class SemLock {
  // See "DESCRIPTION/Named semaphores" at
  // https://man7.org/linux/man-pages/man7/sem_overview.7.html
  static constexpr size_t MAX_SEM_NAME_LEN = NAME_MAX - 4;

public:
  SemLock(std::string_view name) {
    // Validate name
    if (name.size() < 1 || name.size() > MAX_SEM_NAME_LEN) {
      throw std::length_error(
          std::format("({}:{}) Semaphore name \"{}\" of length {} is invalid: "
                      "max name length for a semaphore is {}",
                      __FILE__, __LINE__, name, name.size(), MAX_SEM_NAME_LEN));
    }

    // Copy name
    m_Name = new char[name.size() + 1]{};
    std::strncpy(m_Name, name.data(), name.size());

    // Try to create the semaphore
    sem_t *sem = sem_open(m_Name, O_CREAT | O_EXCL, 0600, 1);
    if (sem == SEM_FAILED) {
      // Failed
      if (errno == EEXIST) {
        // Already existed - just open it
        sem = sem_open(m_Name, 0, 0600);
      }

      if (sem == SEM_FAILED) {
        // Failed again
        const int err = errno;
        throw std::runtime_error(
            std::format("({}:{}) Failed to open semaphore {}: {}", __FILE__,
                        __LINE__, m_Name, strerror(err)));
      }
    }

    m_Sem = sem;
  }

  ~SemLock() {
    const int ret = sem_close(m_Sem);
    if (ret == -1) {
      // Failed - print error
      const int err = errno;
      std::cerr << std::format("({}:{}) Failed to close semaphore {}: {}",
                               __FILE__, __LINE__, m_Name, strerror(err))
                << std::endl;
    }

    delete[] m_Name;
  }

  EXPLICIT_DELETE_CONSTRUCTORS(SemLock);

  bool Acquire() { return sem_trywait(m_Sem) == -1 ? false : true; }
  bool Release() { return sem_post(m_Sem) == -1 ? false : true; }

  bool Acquire(int &errc) {
    const int ret = sem_trywait(m_Sem);
    if (ret == -1) {
      errc = errno;
      return false;
    }

    return true;
  }

  bool Release(int &errc) {
    const int ret = sem_post(m_Sem);
    if (ret == -1) {
      errc = errno;
      return false;
    }

    return true;
  }

private:
  char *m_Name{nullptr};
  sem_t *m_Sem{nullptr};
};
