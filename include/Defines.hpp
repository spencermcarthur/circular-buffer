#pragma once

#define DELETE_DEFAULT_CONSTRUCTORS(CLS)                                       \
  CLS() = delete;                                                              \
  CLS(const CLS &) = delete;                                                   \
  CLS &operator=(const CLS &) = delete;                                        \
  CLS(CLS &&) = delete;                                                        \
  CLS &operator=(CLS &&) = delete

#ifndef CACHELINE_SIZE_BYTES
#define CACHELINE_SIZE_BYTES 64
#endif
