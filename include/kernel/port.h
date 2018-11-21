#ifndef PORT_H
#define PORT_H
#include<stdint.h>
#include <stddef.h>


static inline uint8_t portByteIn(uint16_t port) {
	uint8_t result;
	__asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
	return result;
}

static inline void portByteOut(uint16_t port, uint8_t data) {
	uint16_t port16 = (uint16_t)(port & 0xFFFF);
	uint8_t  data8  = (uint8_t) (data & 0xFF); 
	__asm__ __volatile__("out %%al, %%dx" : : "a" (data8), "d" (port16));
}

static inline uint16_t portWordIn(uint16_t port) {
	uint16_t result;
	__asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
	return result;
}

static inline void portWordOut(uint16_t port, uint16_t data) {
	__asm__ __volatile__("out %%ax, %%dx" : : "a" (data), "d" (port));
}


static inline void insw(uint16_t port, uint16_t* data, size_t count) {
	asm volatile("cld;rep insw"
				 : : 
				 "D" (data), 
				 "d" (port),
				 "c" (count)
				);
}
#endif
