#pragma once

#define EXPLICIT_DELETE_CONSTRUCTORS(className)       \
    className() = delete;                             \
    className(const className &) = delete;            \
    className &operator=(const className &) = delete; \
    className(className &&) = delete;                 \
    className &operator=(className &&) = delete

#define EXPLICIT_DELETE_COPY_MOVE(className)          \
    className(const className &) = delete;            \
    className &operator=(const className &) = delete; \
    className(className &&) = delete;                 \
    className &operator=(className &&) = delete

#ifndef _MAX_SHARED_MEM_SIZE_MIB
#define _MAX_SHARED_MEM_SIZE_MIB 50
#endif

#ifndef _MAX_MESSAGE_SIZE_BYTES
#define _MAX_MESSAGE_SIZE_BYTES 65535
#endif

#ifndef _CACHELINE_SIZE_BYTES
#define _CACHELINE_SIZE_BYTES 64
#endif

#define CONSTEXPR_SV constexpr std::string_view