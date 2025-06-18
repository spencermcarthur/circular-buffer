#pragma once

#define DELETE_DEFAULT_CONSTRUCTORS(CLS)                                       \
  CLS() = delete;                                                              \
  CLS(const CLS &) = delete;                                                   \
  CLS &operator=(const CLS &) = delete;                                        \
  CLS(CLS &&) = delete;                                                        \
  CLS &operator=(CLS &&) = delete

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif
