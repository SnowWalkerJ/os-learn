#include <kernel/port.h>
#include <kernel/memory.h>
#include <drivers/screen.h>
#include <libs/string.h>

uint8_t portByteIn(uint16_t port) {
	uint8_t result;
	__asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
	return result;
}

void portByteOut(uint16_t port, uint8_t data) {
	uint16_t port16 = (uint16_t)(port & 0xFFFF);
	uint8_t  data8  = (uint8_t) (data & 0xFF); 
	__asm__ __volatile__("out %%al, %%dx" : : "a" (data8), "d" (port16));
}

uint16_t portWordIn(uint16_t port) {
	uint16_t result;
	__asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
	return result;
}

void portWordOut(uint16_t port, uint16_t data) {
	__asm__ __volatile__("out %%ax, %%dx" : : "a" (data), "d" (port));
}


void insw(uint16_t port, uint16_t* data, size_t count) {
	asm volatile("rep insw"
				 : : 
				 "D" (data), 
				 "d" (port),
				 "c" (count)
				);
}