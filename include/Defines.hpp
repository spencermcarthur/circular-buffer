#pragma once

#define EXPLICIT_DELETE_CONSTRUCTORS(className)       \
    className() = delete;                             \
    className(const className &) = delete;            \
    className &operator=(const className &) = delete; \
    className(className &&) = delete;                 \
    className &operator=(className &&) = delete

#ifndef CACHELINE_SIZE_BYTES
#define CACHELINE_SIZE_BYTES 64
#endif
