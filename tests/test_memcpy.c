#include <stddef.h>
#include <libs/assert.h>
#include <libs/stdlib.h>
#include <libs/string.h>


char* test_memcpy() {
    char dst[13];
    char src[] = "Hello, world";
    memcpy(src, dst, strlen(src)+1);
    if (strcmp(src, dst) != 0) return "string not same";
    return NULL;
}