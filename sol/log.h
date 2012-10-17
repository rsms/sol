#ifndef S_LOG_H_
#define S_LOG_H_
#include <sol/common.h>

#ifndef S_LOG_STREAM
#define S_LOG_STREAM stderr
#endif

//
// void SLog(const char* format, ...)  -- log a message
// void SLogT()                        -- log current function*
// void SLogD(const char* format, ...) -- log a debugging message*
// void SLogW(const char* format, ...) -- log a warning message
// void SLogE(const char* format, ...) -- log an error message
//
// * = excluded in release builds
//
#if S_DEBUG
  #define STrace(fmt, ...) SLog_("-", "0;36", 1, "%s", __PRETTY_FUNCTION__)
  #define SLogD(fmt, ...)  SLog_("D", "0;34", 0, fmt, ##__VA_ARGS__)
#else
  #define STrace() ((void)0)
  #define SLogD(...) ((void)0)
#endif
#define SLogW(fmt, ...)    SLog_("W", "0;33", 0, fmt, ##__VA_ARGS__)
#define SLogE(fmt, ...)    SLog_("E", "0;31", 0, fmt, ##__VA_ARGS__)

#define SLog(fmt, ...)     SLog_(0,   0,      0, fmt, ##__VA_ARGS__)

// -----

#define SLog_(LN, style, fmt, ...) \
  SLog__(__FILE__, __LINE__, LN, style, fmt, ##__VA_ARGS__)

void SLog__(
  const char *filename,
  int line,
  const char *prefix,
  const char *prefix_style,
  bool style_whole_line,
  const char *format,
  ...);

#endif // S_LOG_H_
