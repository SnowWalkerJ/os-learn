#ifndef MEM_H
#define MEM_H
#include <stddef.h>
#include <stdint.h>

void init_malloc();
void *malloc(size_t);
void free(void *);
#endif
