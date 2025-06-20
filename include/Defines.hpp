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

#ifndef __MAX_SHARED_MEM_SIZE_MIB
#define __MAX_SHARED_MEM_SIZE_MIB 50
#endif

#ifndef __MAX_MESSAGE_SIZE_BYTES
#define __MAX_MESSAGE_SIZE_BYTES 65535
#endif

#ifndef __CACHELINE_SIZE_BYTES
#define __CACHELINE_SIZE_BYTES 64
#endif

#define CONSTEXPR_SV constexpr std::string_view