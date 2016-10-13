/* log.c - debug & error logging
 * Copyright 2012 Keath Milligan
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <log/log.h>

void _log_func(void (*output_func)(const char *msg),
               const char *file,
               const char *function,
               int line,
               const char *fmt, ...) {
    char buf[256];
    int len;
#ifdef WIN32
    const char *p = strrchr(file, '\\');
#else
    const char *p = strrchr(file, '/');
#endif
    if (p != NULL)
        p++;
    if (p == NULL || *p == '\0')
        p = file;
    va_list a;
    va_start(a, fmt);
    len = snprintf(buf, sizeof(buf), "%s:%s:%d: ", p, function, line);
    if (len >= sizeof(buf)) {
        _log_err_output("log line too long\n");
    } else {
        len = vsnprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), fmt, a);
        if (buf[strlen(buf)-1] != '\n')
            buf[strlen(buf)-1] = '\n';
        output_func(buf);
    }
    va_end(a);
}
