#include <kernel/memory.h>
#include <libs/stdlib.h>
#include <libs/string.h>

char *int_to_hex(uint32_t value) {
    char *x = (char *)malloc(9 * sizeof(char));
    memcpy(x, "00000000", 9 * sizeof(char));
    unsigned char *v = (unsigned char *)&value;
    for (int i = 0; i < 4; i++) {
        char c = v[i];
        char h = ((c >> 4) & 0xf) + '0';
        if (h > '9')
            h += 'A' - '9' - 1;
        char l = (c & 0xf) + '0';
        if (l > '9')
            l += 'A' - '9' - 1;
        *(x + 2 * (3 - i))     = h;
        *(x + 2 * (3 - i) + 1) = l;
    }
    return x;
}

char *itoa(uint32_t value, uint32_t digits) {
    char *x      = (char *)malloc(13 * sizeof(char));
    uint32_t len = 1;
    for (uint32_t m = value; m /= 10; len++)
        ;
    len = len > digits ? len : digits;
    for (int i = 0; i < (int)len; i++) {
        x[len - i - 1] = (value % 10) + '0';
        value /= 10;
    }
    x[len] = 0;
    return x;
}

size_t strlen(const char *string) {
    unsigned int i;
    for (i = 0; string[i] != 0; i++)
        ;
    return i;
}

int strcmp(const char *str1, const char *str2) {
    for (; !(*str1 == 0 || *str2 == 0 || *str1 != *str2); str1++, str2++)
        ;
    return (*str1 - *str2);
}

int strncmp(const char *str1, const char *str2, size_t n) {
    int result;
    for (size_t i = 0; i < n; i++) {
        result = (int)(str1[i] - str2[i]);
        if (result == 0)
            continue;
        return result;
    }
    return result;
}