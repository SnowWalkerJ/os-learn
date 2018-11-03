#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <kernel/port.h>
#include <kernel/isr.h>

void keyboard_callback(registers_t*);
void set_input_mask(char);

#endif
