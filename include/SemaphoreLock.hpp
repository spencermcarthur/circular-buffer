#pragma once

#include <linux/limits.h>
#include <semaphore.h>

#include <atomic>
#include <cstddef>
#include <string_view>

#include "Defines.hpp"

// Semaphore lock class
class SemaphoreLock {
public:
    // See "DESCRIPTION/Named semaphores" at
    // https://man7.org/linux/man-pages/man7/sem_overview.7.html
    static constexpr size_t MAX_SEMAPHORE_NAME_LEN = NAME_MAX - 4;

    explicit SemaphoreLock(std::string_view name);
    ~SemaphoreLock();

    EXPLICIT_DELETE_CONSTRUCTORS(SemaphoreLock);

    bool Acquire() noexcept;
    bool Acquire(int &err) noexcept;
    bool Release() noexcept;
    bool Release(int &err) noexcept;

    [[nodiscard]] std::string_view Name() const { return m_Name; }

private:
    char *m_Name{nullptr};
    sem_t *m_Sem{nullptr};
    std::atomic_bool m_HoldsOwnership{false};
};
