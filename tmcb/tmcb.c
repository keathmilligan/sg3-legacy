/* tmcb.h - Tracked memory control blocks
 * Tracks malloc'd blocks
 * Copyright 2012 Keath Milligan
 */

#include <stdio.h>
#include <string.h>
#include <log/log.h>
#include <ut/uthash.h>
#include "tmcb.h"

#undef malloc
#undef calloc
#undef realloc
#undef free
#undef strdup
#undef strndup

long total_allocs = 0;
long total_frees = 0;

void *tmalloc(const char *file, int line, size_t size) {
    void *tmp = malloc(size);
    if (tmp == NULL) {
        LOGERRX(file, "", line, "malloc failed (request %d bytes)\n", size);
        return NULL;
    } else {
        TMCB *cb = malloc(sizeof(TMCB));
        if (cb == NULL) {
            LOGERRX(file, "", line, "failed to allocate TMCB\n");
        } else {
        	total_allocs++;
            cb->block = tmp;
            cb->file = strdup(file);
            cb->line = line;
            cb->size = size;
            HASH_ADD_PTR(TMCB_list, block, cb);
        }
        return tmp;
    }
}

void *trealloc(const char *file, int line, void *block, size_t size) {
    TMCB *cb = NULL;
    void *tmp;
    tmp = realloc(block, size);
    HASH_FIND_PTR(TMCB_list, &block, cb);
    if (cb != NULL) HASH_DEL(TMCB_list, cb);
    if (tmp == NULL) {
        LOGERRX(file, "", line, "realloc of block %x failed (request %d bytes)\n", (unsigned int)block, size);
        if (cb != NULL) {
            free(cb->file);
            free(cb);
        }
    } else {
        if (cb == NULL) cb = malloc(sizeof(TMCB));
        cb->block = tmp;
        cb->size = size;
        cb->file = strdup(file);
        cb->line = line;
        HASH_ADD_PTR(TMCB_list, block, cb);
    }
    return tmp;
}

void *tcalloc(const char *file, int line, size_t count, size_t eltsize) {
    size_t size = count*eltsize;
    TMCB *cb = malloc(sizeof(TMCB));
    cb->block = calloc(count, eltsize);
    if (cb->block == NULL) {
        LOGERRX(file, "", line, "cmalloc failed (request %d bytes)\n", size);
        free(cb);
        return NULL;
    } else {
    	total_allocs++;
        cb->size = size;
        cb->file = strdup(file);
        cb->line = line;
        HASH_ADD_PTR(TMCB_list, block, cb);
        return cb->block;
    }
}

void tfree(const char *file, int line, void *block) {
    TMCB *cb;
    if (block != NULL) {
        HASH_FIND_PTR(TMCB_list, &block, cb);
        if (cb != NULL) {
            HASH_DEL(TMCB_list, cb);
            free(cb->file);
            free(cb);
            total_frees++;
        } else {
            LOGERRX(file, "", line, "TMCB not found for block: %x\n", (unsigned int)block);
        }
        free(block);
    } else {
        LOGERRX(file, "", line, "refusing to free NULL block\n");
    }
}

void tcheck() {
    int count = HASH_COUNT(TMCB_list);
    TMCB *cb, *tmp;
    LOG("Total allocs: %ld, total frees: %ld\n", total_allocs, total_frees);
    if (count > 0) {
        LOGERR("Memory leaks detected: %d blocks\n", count);
        HASH_ITER(hh, TMCB_list, cb, tmp) {
            LOGERRX(cb->file, "", cb->line, "%x %d bytes\n", (unsigned int)cb->block, cb->size);
        }
    }
}

char *tstrdup(const char *file, int line, const char *s) {
    char *tmp = tmalloc(file, line, strlen(s)+1);
    if (tmp != NULL) strcpy(tmp, s);
    return tmp;
}

char *tstrndup(const char *file, int line, const char *s, size_t size) {
    char *tmp = tmalloc(file, line, strlen(s) <= size? strlen(s)+1 : size+1);
    if (tmp != NULL) strcpy(tmp, s);
    return tmp;
}

