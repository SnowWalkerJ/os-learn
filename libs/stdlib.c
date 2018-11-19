#include <libs/stdlib.h>

void memsetb(void* addr, uint8_t value, size_t count) {
    asm("rep;"
        "stosb;"::
        "D"(addr),
        "a"(value),
        "c"(count));
}

void memsetw(void* addr, uint16_t value, size_t count) {
    asm("rep;"
        "stosw;"::
        "D"(addr),
        "a"(value),
        "c"(count));
}

void memcpy(void* src, void* dst, size_t count) {
    if (src < dst) {
        dst = (void*)((char*)dst + count - 1);
        src = (void*)((char*)src + count) - 1;
        asm volatile("std;"
        "rep movsb" ::
        "S" (src),
        "D" (dst),
        "c" (count));
    } else {
        asm volatile("cld;"
        "rep movsb" ::
        "S" (src),
        "D" (dst),
        "c" (count));
    }
}
