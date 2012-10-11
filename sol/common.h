#ifndef S_COMMON_H_
#define S_COMMON_H_
#define S_COMMON_H_INSIDE_

#define S_STR1(str) #str
#define S_STR(str) S_STR1(str)

#define S_VERSION_MAJOR 0
#define S_VERSION_MINOR 1
#define S_VERSION_BUILD 0
#define S_VERSION_STRING \
  S_STR(S_VERSION_MAJOR) "." S_STR(S_VERSION_MINOR) "." S_STR(S_VERSION_BUILD)

#ifndef S_DEBUG
  #define S_DEBUG 0
#endif

#include <sol/common_target.h>

#ifdef __BASE_FILE__
  #define S_FILENAME __BASE_FILE__
#else
  #define S_FILENAME ((strrchr(__FILE__, '/') ?: __FILE__ - 1) + 1)
#endif

#define S_countof(a) (sizeof(a)/sizeof(*(a)))

// Attributes
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
  #define S_UNUSED __attribute__((unused))
#else
  #define S_UNUSED
#endif
#if __has_attribute(pure)
  #define S_PURE __attribute__((pure))
#else
  #define S_PURE
#endif
#if __has_attribute(warn_unused_result)
  #define S_WUNUSEDR __attribute__((warn_unused_result))
#else
  #define S_WUNUSEDR
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

#include <sol/common_stdint.h> // .. include <std{io,int,def,bool}>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#if S_HOST_OS_POSIX
  #include <unistd.h>
#endif

#undef S_COMMON_H_INSIDE_
#endif // S_COMMON_H_
