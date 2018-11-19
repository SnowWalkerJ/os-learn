#ifndef PORT_H
#define PORT_H
#include<stdint.h>
#include <stddef.h>


uint8_t portByteIn(uint16_t);
uint16_t portWordIn(uint16_t);
void portByteOut(uint16_t, uint8_t);
void portWordOut(uint16_t, uint16_t);
void insw(uint16_t port, uint16_t* data, size_t count);
#endif
