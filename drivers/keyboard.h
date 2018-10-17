#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../cpu/port.h"
#include "../cpu/isr.h"

void keyboard_callback(registers_t*);
void set_input_mask(char);

#endif
