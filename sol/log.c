#include "log.h"
#include <unistd.h> // isatty
#include <stdarg.h> // va_*

FILE* SLogStream = 0;

void SLog__(
    const char *filename,
    int line,
    const char *prefix,
    const char *prefix_style,
    bool style_whole_line,
    const char *format,
    ...) {

  if (SLogStream == 0) {
    SLogStream = stderr;
  }

  static int color_output = -1;
  if (color_output == -1) {
    color_output = !!isatty(1);
  }

  flockfile(SLogStream);

  if (style_whole_line && color_output) {
    if (prefix) {
      fprintf(SLogStream, "\e[%sm%s ", prefix_style, prefix ? prefix : "");
    } else {
      fprintf(SLogStream, "\e[%sm", prefix_style);
    }
  } else if (prefix) {
    if (color_output) {
      fprintf(SLogStream, "\e[%sm%s\e[0m ", prefix_style, prefix);
    } else {
      fprintf(SLogStream, "%s ", prefix);
    }
  }

  va_list ap;
  va_start(ap, format);
  vfprintf(SLogStream, format, ap);
  va_end(ap);
  
  fprintf(
    SLogStream,
    color_output ? " \e[30;1m(%s:%d)\e[0m\n" : " (%s:%d)",
    filename,
    line
  );

  funlockfile(SLogStream);
  fflush(SLogStream);
}