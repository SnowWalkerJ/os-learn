#include <kernel/isr.h>
#include <drivers/keyboard.h>

void register_interrupt_handlers () {
    register_interrupt_handler(IRQ1, keyboard_callback);
}
