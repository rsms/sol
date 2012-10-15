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

// Terminate process with status 70 (EX_SOFTWARE), writing `fmt` with optional
// arguments to stderr.
#define S_FATAL(fmt, ...) \
  errx(70, "FATAL: " fmt " at " __FILE__ ":" S_STR(__LINE__), ##__VA_ARGS__)

// Print `fmt` with optional arguments to stderr, and abort() process causing
// EXC_CRASH/SIGABRT.
#define S_CRASH(fmt, ...) do { \
    fprintf(stderr, "CRASH: " fmt " at " __FILE__ ":" S_STR(__LINE__) "\n", \
      ##__VA_ARGS__); \
    abort(); \
  } while (0)

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
    assert(!"Declared S_UNREACHABLE but was reached"); \
    __builtin_unreachable(); \
  } while(0)
#else
  #define S_UNREACHABLE \
    assert(!"Declared S_UNREACHABLE but was reaced")
#endif

#define S_NOT_IMPLEMENTED S_FATAL("NOT IMPLEMENTED in %s", __PRETTY_FUNCTION__)

#define S_MAX(a,b) \
  ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define S_MIN(a,b) \
  ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#include <sol/common_stdint.h> // .. include <std{io,int,def,bool}>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <err.h>

#if S_HOST_OS_POSIX
  #include <unistd.h>
#endif

#undef S_COMMON_H_INSIDE_
#endif // S_COMMON_H_
