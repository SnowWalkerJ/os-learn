#ifndef CMOS_H
#define CMOS_H
#include <cpu/port.h>
#include <stdint.h>

#define READ_CMOS(addr) ({portByteOut(0x70, (addr)|0x80); portByteIn(0x71);})
#define BCD_TO_BIN(val) (((val) & 0xf) + (((val) >> 4) * 10))
#endif