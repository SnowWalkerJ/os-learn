#ifndef RTC_H
#define RTC_H
#include <kernel/port.h>
#include <stdint.h>

#define BCD_TO_BIN(val) (((val) & 0xf) + (((val) >> 4) * 10))

uint8_t read_rtc_register(uint32_t);
#endif