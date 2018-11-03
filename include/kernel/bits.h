#ifndef BITS_H
#define BITS_H

#include<stddef.h>
#define low_16(value) (uint16_t)((value) & 0xffff)
#define high_16(value) (uint16_t)(((value) >> 16) & 0xffff)

#endif
