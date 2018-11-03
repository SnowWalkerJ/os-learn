#ifndef PS2_H
#define PS2_H

#include <stdint.h>
#define PS2_CMD 0x64
#define PS2_STATUS 0x64
#define PS2_DATA 0x60

uint8_t send_ps2_command(uint8_t cmd);
int test_first_ps2_port();

#endif
