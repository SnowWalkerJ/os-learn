#ifndef PORT_H
#define PORT_H
#include<stdint.h>


uint8_t portByteIn(uint16_t);
uint16_t portWordIn(uint16_t);
void portByteOut(uint16_t, uint8_t);
void portWordOut(uint16_t, uint16_t);
#endif
