#include "log.h"
#include <unistd.h> // isatty
#include <stdarg.h> // va_*

void SLog__(
    const char *filename,
    int line,
    const char *prefix,
    const char *prefix_style,
    const char *format,
    ...) {
  static int color_output = -1;
  if (color_output == -1) {
    color_output = !!isatty(1);
  }
  if (prefix) {
    if (color_output) {
      fprintf(S_LOG_STREAM, "\e[%sm%s\e[0m ", prefix_style, prefix);
    } else {
      fprintf(S_LOG_STREAM, "%s ", prefix);
    }
  }

  va_list ap;
  va_start(ap, format);
  vfprintf(S_LOG_STREAM, format, ap);
  va_end(ap);
  
  fprintf(
    S_LOG_STREAM,
    color_output ? " \e[30;1m(%s:%d)\e[0m\n" : " (%s:%d)",
    filename,
    line
  );
}