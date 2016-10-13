/* tmcb.h - Tracked memory control blocks
 * Tracks malloc'd blocks
 * Copyright 2012 Keath Milligan
 */


#ifndef TMCB_H_
#define TMCB_H_

#include <ut/uthash.h>

typedef struct _TMCB {
    void *block;
    int size;
    char *file;
    int line;
    UT_hash_handle hh;
} TMCB;

TMCB *TMCB_list;

void tcheck();

#endif /* TMCB_H_ */
