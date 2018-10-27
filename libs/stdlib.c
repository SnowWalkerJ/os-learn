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
    if (src < dst)
        asm("std");
    else
        asm("cld");
    asm("mov %0, %%edi;"
        "mov %1, %%esi;"
        "rep;"
        "movsb" ::
        "r" (src),
        "r" (dst),
        "c" (count));
}