#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdio.h>

#ifndef LOG_LEVEL
#define LOG_LEVEL 0
#endif

enum log_level {
    LOG_ERROR = 0,
    LOG_INFO,
    LOG_DEBUG,
};

static inline void
llog(enum log_level level, char *fmt, ...) {
    if(level > LOG_LEVEL)
        return;

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    putchar('\n');
}

#endif
