#include <libs/stdlib.h>

void memsetb(void *addr, uint8_t value, size_t count) {
    asm volatile("cld;"
                 "rep;"
                 "stosb;" ::"D"(addr),
                 "a"(value), "c"(count));
}

void memsetw(void *addr, uint16_t value, size_t count) {
    // for (size_t i = 0; i < count; i++) {
    //     ((uint16_t*)addr)[i] = value;
    // }
    asm volatile("cld;"
                 "rep;"
                 "stosw;" ::"D"(addr),
                 "a"(value), "c"(count));
}

void memsetd(void *addr, uint32_t value, size_t count) {
    // for (size_t i = 0; i < count; i++) {
    //     ((uint16_t*)addr)[i] = value;
    // }
    asm volatile("cld;"
                 "rep;"
                 "stosl;" ::"D"(addr),
                 "a"(value), "c"(count));
}

void memcpy(void *dst, const void *src, size_t count) {
    if (src < dst) {
        dst = (void *)((char *)dst + count - 1);
        src = (void *)((char *)src + count) - 1;
        asm volatile("std;"
                     "rep movsb" ::"S"(src),
                     "D"(dst), "c"(count));
    } else {
        asm volatile("cld;"
                     "rep movsb" ::"S"(src),
                     "D"(dst), "c"(count));
    }
}
