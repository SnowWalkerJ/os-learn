#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../cpu/port.h"
#include "../cpu/isr.h"

void init_keyboard();
void set_input_mask(char);

#endif
