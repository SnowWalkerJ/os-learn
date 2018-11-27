#include <drivers/keyboard.h>
#include <kernel/isr.h>

void register_interrupt_handlers() {
    register_interrupt_handler(IRQ1, keyboard_callback);
}
