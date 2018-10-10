#include "port.h"


uint8_t portByteIn(uint16_t port) {
	uint8_t result;
	__asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
	return result;
}

void portByteOut(uint16_t port, uint8_t data) {
	__asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

uint16_t portWordIn(uint16_t port) {
	uint16_t result;
	__asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
	return result;
}

void portWordOut(uint16_t port, uint16_t data) {
	__asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}
