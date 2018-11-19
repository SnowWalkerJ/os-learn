#ifndef STRING_H
#define STRING_H
#include <stdint.h>
#include <stddef.h>
char* int_to_hex(uint32_t);
char* int_to_oct(uint32_t, uint32_t);
unsigned int strlen(char*);
int strcmp(char*, char*);
int strncmp(char*, char*, size_t);
#endif /* STRING_H */