#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>
#include <stddef.h>

void memsetb(void* addr, uint8_t value, size_t count);

void memsetw(void* addr, uint16_t value, size_t count);

void memcpy(void* src, void* dst, size_t count);
#endif /* STDLIB_H */