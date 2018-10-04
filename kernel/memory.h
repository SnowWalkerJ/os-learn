#ifndef MEM_H
#define MEM_H
#include <stddef.h>
#include <stdint.h>
void initMemTable();
void* malloc(size_t);
int free(void*);
void memcpy(void*, void*, size_t, int);
#endif
