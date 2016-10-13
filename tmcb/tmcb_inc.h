/* tmcb_inc.h - common include file
 * Include this file in every module with -include.
 * Copyright 2012 Keath Milligan
 */

#ifndef TMCB_INC_H_
#define TMCB_INC_H_

#include <stdlib.h>
#include <string.h>

void *tmalloc(const char *file, int line, size_t size);
void *tcalloc(const char *file, int line, size_t count, size_t size);
void *trealloc(const char *file, int line, void *block, size_t size);
void tfree(const char *file, int line, void *block);
char *tstrdup(const char *file, int line, const char *s);
char *tstrndup(const char *file, int line, const char *s, size_t size);

#undef malloc
#undef calloc
#undef realloc
#undef free
#undef strdup
#undef strndup

// use our custom memory-management functions
#define malloc(x) tmalloc(__FILE__, __LINE__, x)
#define realloc(x, s) trealloc(__FILE__, __LINE__, x, s)
#define calloc(n, s) tcalloc(__FILE__, __LINE__, n, s)
#define free(x) tfree(__FILE__, __LINE__, x)
#define strdup(x) tstrdup(__FILE__, __LINE__, x)
#define strndup(x, s) tstrndup(__FILE__, __LINE__, x, s)

#endif /* TMCB_INC_H_ */
