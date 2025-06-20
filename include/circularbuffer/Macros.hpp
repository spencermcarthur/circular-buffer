#pragma once

#define CB_EXPLICIT_DELETE_CONSTRUCTORS(className)       \
    className() = delete;                             \
    className(const className &) = delete;            \
    className &operator=(const className &) = delete; \
    className(className &&) = delete;                 \
    className &operator=(className &&) = delete

#define CB_EXPLICIT_DELETE_COPY_MOVE(className)          \
    className(const className &) = delete;            \
    className &operator=(const className &) = delete; \
    className(className &&) = delete;                 \
    className &operator=(className &&) = delete

#ifndef CB_MAX_SHARED_MEM_SIZE_MIB
#define CB_MAX_SHARED_MEM_SIZE_MIB 50
#endif

#ifndef CB_MAX_MESSAGE_SIZE_BYTES
#define CB_MAX_MESSAGE_SIZE_BYTES 65535
#endif

#ifndef CB_CACHELINE_SIZE_BYTES
#define CB_CACHELINE_SIZE_BYTES 64
#endif

#define CB_CONSTEXPR_SV constexpr std::string_view
