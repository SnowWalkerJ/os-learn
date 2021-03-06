#include <stdint.h>
#include <drivers/rtc.h>
#include <kernel/port.h>
#include <kernel/x86.h>

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71


uint8_t read_rtc_register(uint32_t reg) {
    outb(CMOS_ADDR, reg|0x80);
    return inb(CMOS_DATA);
}