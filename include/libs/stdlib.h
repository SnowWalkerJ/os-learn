#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>
#include <stdint.h>

void memsetb(void *addr, uint8_t value, size_t count);

void memsetw(void *addr, uint16_t value, size_t count);

void memsetd(void *addr, uint32_t value, size_t count);

void memcpy(void *dst, const void *src, size_t count);
#endif /* STDLIB_H */