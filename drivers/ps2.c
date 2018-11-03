#include <drivers/ps2.h>
#include <kernel/port.h>

uint8_t send_ps2_command(uint8_t cmd) {
	portByteOut(PS2_CMD, cmd);
	return (uint8_t)portByteIn(PS2_DATA);
}

int test_first_ps2_port() {
	return send_ps2_command(0xAB) == 0;
}
