// Helpers for tests
#ifndef S_TEST_TEST_H_
#define S_TEST_TEST_H_
#include <sol/common.h>

#if S_TEST_SUIT_RUNNING
  #define print(...) ((void)0)
#else
  #define print(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#endif

#endif  // S_TEST_TEST_H_
