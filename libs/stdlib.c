#include "stdlib.h"

void memsetb(void* addr, uint8_t value, size_t count) {
    asm("mov %0, %%edi;"
        "rep;"
        "stosb;"::
        "r"(addr),
        "a"(value),
        "c"(count));
}

void memsetw(void* addr, uint16_t value, size_t count) {
    asm("mov %0, %%edi;"
        "rep;"
        "stosw;"::
        "r"(addr),
        "a"(value),
        "c"(count));
}

void memcpy(void* src, void* dst, size_t count) {
    if (src < dst) {
        dst = (void*)((char*)dst + count - 1);
        src = (void*)((char*)src + count) - 1;
        asm volatile("std;"
        "mov %0, %%esi;"
        "mov %1, %%edi;"
        "rep movsb" ::
        "r" (src),
        "r" (dst),
        "c" (count));
    } else {
        asm volatile("cld;"
        "mov %0, %%esi;"
        "mov %1, %%edi;"
        "rep movsb" ::
        "r" (src),
        "r" (dst),
        "c" (count));
    }
}
