#ifndef CRX_H
#define CRX_H

#include <stdint.h>
extern uint32_t read_cr0();
extern uint32_t read_cr1();
extern uint32_t read_cr2();
extern uint32_t read_cr3();

extern void write_cr0(uint32_t);
extern void write_cr1(uint32_t);
extern void write_cr2(uint32_t);
extern void write_cr3(uint32_t);
#endif
