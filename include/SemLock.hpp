#pragma once

#include <linux/limits.h>
#include <semaphore.h>

#include <atomic>
#include <cstddef>
#include <string_view>

#include "Defines.hpp"

// Semaphore lock class
class SemLock {
    static constexpr int OPEN_MODE_RDWR = 0600;

public:
    // See "DESCRIPTION/Named semaphores" at
    // https://man7.org/linux/man-pages/man7/sem_overview.7.html
    static constexpr size_t MAX_SEM_NAME_LEN = NAME_MAX - 4;

    explicit SemLock(std::string_view name);
    ~SemLock();

    EXPLICIT_DELETE_CONSTRUCTORS(SemLock);

    bool Acquire() noexcept;
    bool Acquire(int &err) noexcept;
    bool Release() noexcept;
    bool Release(int &err) noexcept;

private:
    char *m_Name{nullptr};
    sem_t *m_Sem{nullptr};
    std::atomic_bool m_HoldsOwnership{false};
};
