/* log.h - debug & error logging
 * Copyright 2012 Keath Milligan
 */

#ifndef LOG_H_
#define LOG_H_

#define LOG(...) _log_func(_log_std_output, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOGERR(...) _log_func(_log_err_output, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOGX(file, function, line, ...) _log_func(_log_std_output, file, function, line, __VA_ARGS__)
#define LOGERRX(file, function, line, ...) _log_func(_log_err_output, file, function, line, __VA_ARGS__)

void _log_std_output(const char *msg);
void _log_err_output(const char *msg);

void _log_func(void (*output_func)(const char *msg), const char *file, const char *function, int line, const char *fmt, ...);

#endif /* LOG_H_ */
