#include <libs/string.h>
#include <fs/bitmap.h>
#include <libs/stdlib.h>


void empty_bitmap(struct bitmap* bm) {
    /* Set all bits in the area to zero */
    memsetb(bm->map, 0, bm->size);
}


void set_one(struct bitmap* bm, unsigned int addr) {
    unsigned int byte_addr = addr / 8;
    unsigned int bit_addr = addr % 8;
    char* offset = bm->map + byte_addr;
    *offset |= (char)(1 << (7 - bit_addr));
}


void set_zero(struct bitmap* bm, unsigned int addr) {
    unsigned int byte_addr = addr / 8;
    unsigned int bit_addr = addr % 8;
    char* offset = bm->map + byte_addr;
    *offset &= ~(char)(1 << (7 - bit_addr));
}


unsigned int first_zero(struct bitmap* bm) {
    unsigned int result;
    asm volatile(
        "cld;"
        "1: lodsl;"              // [esi] -> eax
        "notl %%eax;"
        "bsfl %%eax, %%edx;"
        "je 2f;"
        "add %%edx, %%ecx;"
        "jmp 3f;"
        "2: add %%ecx, 32;"
        "cmp %%ebx, %%ecx;"
        "jl 1b;"
        "3:"
        : "=c" (result)
        : "S" (bm->map),
        "c" (0),
        "b" (bm->size*8)
    );
    return result;
}