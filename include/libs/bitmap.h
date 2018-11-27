#ifndef BITMAP_H
#define BITMAP_H
#include <stdint.h>
struct bitmap {
    int size; // size in bytes
    uint8_t *map;
};

void empty_bitmap(struct bitmap *);
void set_one(struct bitmap *, unsigned int);
void set_zero(struct bitmap *, unsigned int);
unsigned int first_zero(struct bitmap *);
#endif