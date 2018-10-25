#include <stdint.h>
#include "rtc.h"
#include <cpu/port.h>
#include <cpu/x86.h>

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71


uint8_t read_rtc_register(uint32_t reg) {
    portByteOut(CMOS_ADDR, reg|0x80);
    return portByteIn(CMOS_DATA);
}