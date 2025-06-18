#include "SemaphoreLock.hpp"

#include <fcntl.h>
#include <semaphore.h>

#include <atomic>
#include <cerrno>
#include <cstring>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string_view>

SemaphoreLock::SemaphoreLock(std::string_view name) {
    // Validate name
    if (name.empty() || name.length() > MAX_SEM_NAME_LEN) {
        throw std::length_error(std::format(
            "({}:{}) Semaphore name \"{}\" of length {} is invalid: "
            "max name length for a semaphore is {}",
            __FILE__, __LINE__, name, name.size(), MAX_SEM_NAME_LEN));
    }

    // Copy name
    m_Name = new char[name.size() + 1]{};
    std::strncpy(m_Name, name.data(), name.size());

    // Try to create the semaphore
    sem_t *sem = sem_open(m_Name, O_CREAT | O_EXCL, OPEN_MODE_RDWR, 1);
    if (sem == SEM_FAILED) {
        // Failed
        if (errno == EEXIST) {
            // Already existed - just open it
            sem = sem_open(m_Name, 0, OPEN_MODE_RDWR);
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

SemaphoreLock::~SemaphoreLock() {
    if (m_HoldsOwnership.load(std::memory_order_acquire)) {
        int err;
        if (!Release(err)) {
            std::cerr << std::format(
                "({}:{}) Failed to release smaphore {}: {}", __FILE__, __LINE__,
                m_Name, strerror(err));
        }
    }

    const int ret = sem_close(m_Sem);
    if (ret == -1) {
        // Failed - print error
        const int err = errno;
        std::cerr << std::format("({}:{}) Failed to close semaphore {}: {}\n",
                                 __FILE__, __LINE__, m_Name, strerror(err));
    }

    delete[] m_Name;
}

bool SemaphoreLock::Acquire() noexcept {
    const bool acquired = sem_trywait(m_Sem) == 0;
    if (acquired) {
        m_HoldsOwnership.store(true, std::memory_order_release);
    }

    return acquired;
}

bool SemaphoreLock::Release() noexcept {
    const bool released = sem_post(m_Sem) == 0;
    if (released) {
        m_HoldsOwnership.store(false, std::memory_order_release);
    }

    return released;
}

bool SemaphoreLock::Acquire(int &err) noexcept {
    const bool acquired = sem_trywait(m_Sem) == 0;
    if (acquired) {
        m_HoldsOwnership.store(true, std::memory_order_release);
    } else {
        err = errno;
    }

    return acquired;
}

bool SemaphoreLock::Release(int &err) noexcept {
    const bool released = sem_post(m_Sem) == 0;
    if (released) {
        m_HoldsOwnership.store(false, std::memory_order_release);
    } else {
        err = errno;
    }

    return true;
}
