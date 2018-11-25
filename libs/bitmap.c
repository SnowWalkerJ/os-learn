#include <libs/string.h>
#include <libs/bitmap.h>
#include <libs/stdlib.h>


void empty_bitmap(struct bitmap* bm) {
    /* Set all bits in the area to zero */
    memsetb(bm->map, 0, bm->size);
}


void set_one(struct bitmap* bm, unsigned int addr) {
    unsigned int byte_addr = addr / 8;
    unsigned int bit_addr = addr % 8;
    uint8_t* offset = bm->map + byte_addr;
    *offset |= (uint8_t)(1 << bit_addr);
}


void set_zero(struct bitmap* bm, unsigned int addr) {
    unsigned int byte_addr = addr / 8;
    unsigned int bit_addr = addr % 8;
    uint8_t* offset = bm->map + byte_addr;
    *offset &= ~(uint8_t)(1 <<  bit_addr);
}


unsigned int first_zero(struct bitmap* bm) {
    uint32_t max_id = bm->size / sizeof(uint32_t);
    for (uint32_t bid = 0; bid < max_id; bid++) {
        uint32_t block = ((uint32_t*)bm->map)[bid];
        if (block != 0xffffffff) {
            int result;
            asm("bsfl %1, %0;"
                : "=r" (result)
                : "r" (~block));
            return bid * 32 + result;
        }
    }
    return bm->size * 8;
}