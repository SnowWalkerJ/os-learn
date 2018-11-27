#ifndef STRING_H
#define STRING_H
#include <stddef.h>
#include <stdint.h>
char *int_to_hex(uint32_t);
char *int_to_oct(uint32_t, uint32_t);
size_t strlen(const char *);
int strcmp(const char *, const char *);
int strncmp(const char *, const char *, size_t);
#endif /* STRING_H */