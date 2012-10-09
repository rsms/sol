#ifndef S_COMMON_H_
#define S_COMMON_H_

#ifndef __has_attribute
  #define __has_attribute(x) 0
#endif
#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif

#if __has_attribute(always_inline)
  #define S_ALWAYS_INLINE __attribute__((always_inline))
#else
  #define S_ALWAYS_INLINE
#endif

#if __has_attribute(deprecated)
  #define S_DEPRECATED __attribute__((deprecated))
#else
  #define S_DEPRECATED
#endif

#if __has_attribute(deprecated)
  #define S_UNUSED __attribute__((unused))
#else
  #define S_UNUSED
#endif

#if __has_builtin(__builtin_unreachable)
  #define S_UNREACHABLE do { \
    assert(!"UNREACHABLE"); \
    __builtin_unreachable(); \
  } while(0)
#else
  #define S_UNREACHABLE \
assert(!"UNREACHABLE")
#endif

#include <sol/stdint.h> // .. include <std{io,int,def,bool}>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define SNil 0

#endif // S_COMMON_H_
