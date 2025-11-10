#ifndef ALLOC_H
#define ALLOC_H

#include <stdio.h>
#include <stdlib.h>

static inline void *
alloc(size_t size) {
    void *p = calloc(1, size);
    if(p == NULL)
        exit(EXIT_FAILURE);

    return p;
}

#endif
