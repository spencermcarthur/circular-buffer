#include "SemaphoreLock.hpp"

#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>

#include <atomic>
#include <cerrno>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string_view>

#include "Macros.hpp"
#include "Utils.hpp"
#include "spdlog/spdlog.h"

static constexpr int OPEN_READ_WRITE = S_IRUSR + S_IWUSR;

SemaphoreLock::SemaphoreLock(const std::string_view name) {
    SetupSpdlog();

    // Validate name
    if (name.empty() || name.length() > MAX_SEMAPHORE_NAME_LEN) {
        CONSTEXPR_SV fmt =
            "({}:{}) Semaphore name \"{}\" of length {} is invalid: length "
            "must >0 and <={}";
        SPDLOG_ERROR(fmt.substr(8), name, name.length(),
                     MAX_SEMAPHORE_NAME_LEN);
        throw std::length_error(std::format(fmt, __FILE__, __LINE__, name,
                                            name.length(),
                                            MAX_SEMAPHORE_NAME_LEN));
    }

    // Copy name
    m_Name = new char[name.size() + 1]{};
    std::strncpy(m_Name, name.data(), name.length());

    // Try to create the semaphore
    sem_t *sem = sem_open(m_Name, O_CREAT | O_EXCL, OPEN_READ_WRITE, 1);
    if (sem == SEM_FAILED) {
        // Failed
        if (errno == EEXIST) {
            // Already existed - just open it
            sem = sem_open(m_Name, 0, OPEN_READ_WRITE);
        }

        if (sem == SEM_FAILED) {
            // Failed again
            const int err = errno;
            CONSTEXPR_SV fmt = "({}:{}) Can't access semaphore {}: {}";
            SPDLOG_ERROR(fmt.substr(8), m_Name, strerror(err));
            throw std::runtime_error(
                std::format(fmt, __FILE__, __LINE__, m_Name, strerror(err)));
        }
    }

    m_Sem = sem;
}

SemaphoreLock::~SemaphoreLock() {
    if (m_HoldsOwnership.load(std::memory_order_acquire)) {
        int err;
        if (!Release(err)) {
            SPDLOG_ERROR("Failed to unlock smaphore {}: {}", m_Name,
                         strerror(err));
        }
    }

    const int ret = sem_close(m_Sem);
    if (ret == -1) {
        // Failed - print error
        const int err = errno;
        SPDLOG_ERROR("Failed to close semaphore {}: {}", m_Name, strerror(err));
    }

    delete[] m_Name;
}

bool SemaphoreLock::Acquire() noexcept {
    const bool acquired = sem_trywait(m_Sem) == 0;
    if (acquired) {
        m_HoldsOwnership.store(true, std::memory_order_release);
        SPDLOG_DEBUG("Locked semaphore {}", m_Name);
    } else {
        SPDLOG_DEBUG("Failed to lock semaphore {}", m_Name);
    }

    return acquired;
}

bool SemaphoreLock::Release() noexcept {
    const bool released = sem_post(m_Sem) == 0;
    if (released) {
        m_HoldsOwnership.store(false, std::memory_order_release);
        SPDLOG_DEBUG("Unlocked semaphore {}", m_Name);
    } else {
        SPDLOG_DEBUG("Failed to unlock semaphore {}", m_Name);
    }

    return released;
}

bool SemaphoreLock::Acquire(int &err) noexcept {
    const bool acquired = sem_trywait(m_Sem) == 0;
    if (acquired) {
        m_HoldsOwnership.store(true, std::memory_order_release);
        SPDLOG_DEBUG("Locked semaphore {}", m_Name);
    } else {
        SPDLOG_DEBUG("Failed to lock semaphore {}", m_Name);
        err = errno;
    }

    return acquired;
}

bool SemaphoreLock::Release(int &err) noexcept {
    const bool released = sem_post(m_Sem) == 0;
    if (released) {
        m_HoldsOwnership.store(false, std::memory_order_release);
        SPDLOG_DEBUG("Unlocked semaphore {}", m_Name);
    } else {
        SPDLOG_DEBUG("Failed to unlock semaphore {}", m_Name);
        err = errno;
    }

    return true;
}
