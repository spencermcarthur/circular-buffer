#pragma once

#define EXPLICIT_DELETE_CONSTRUCTORS(className)       \
    className() = delete;                             \
    className(const className &) = delete;            \
    className &operator=(const className &) = delete; \
    className(className &&) = delete;                 \
    className &operator=(className &&) = delete

#ifndef MAX_SHARED_MEM_SIZE_MIB
#define MAX_SHARED_MEM_SIZE_MIB 50
#endif

#ifndef MAX_MESSAGE_SIZE_BYTES
#define MAX_MESSAGE_SIZE_BYTES 65535
#endif

#ifndef CACHELINE_SIZE_BYTES
#define CACHELINE_SIZE_BYTES 64
#endif

#define CONSTEXPR_SV constexpr std::string_view