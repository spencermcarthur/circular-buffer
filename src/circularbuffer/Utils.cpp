#include "circularbuffer/Utils.hpp"

#include "spdlog/common.h"
#include "spdlog/spdlog.h"

void SetupSpdlog() noexcept {
    // Latch
    static bool setupDone{false};
    if (setupDone) {
        return;
    }

    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%s:%#] [%^%l%$] %v");
#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
    if (spdlog::get_level() != spdlog::level::debug) {
        spdlog::set_level(spdlog::level::debug);
    }
#endif

    setupDone = true;
}