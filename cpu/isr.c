#include <drivers/screen.h>
#include <kernel/idt.h>
#include <kernel/isr.h>
#include <kernel/memory.h>
#include <kernel/port.h>
#include <libs/string.h>
#include <libs/stdio.h>
#include <stddef.h>

#define PIC1_CMD 0x20
#define PIC2_CMD 0xA0
#define PIC1_DATA (PIC1_CMD + 1)
#define PIC2_DATA (PIC2_CMD + 1)
#define PIC_EOI 0x20

static isr_t interrupt_handlers[256];

const char *exceptions_messages[] = {"Division By Zero",
                                     "Debug",
                                     "Non Maskable Interrupt",
                                     "Breakpoint",
                                     "Into Detected Overflow",
                                     "Out of Bounds",
                                     "Invalid Opcode",
                                     "No Coprocessor",

                                     "Double Fault",
                                     "Coprocessor Segment Overrun",
                                     "Bad TSS",
                                     "Segment Not Present",
                                     "Stack Fault",
                                     "General Protection",
                                     "Page Fault",
                                     "Unknown Interrupt",

                                     "Coprocessort Fault",
                                     "Alignment Check",
                                     "Machine Check",
                                     "Reserved",
                                     "Reserved",
                                     "Reserved",
                                     "Reserved",
                                     "Reserved",

                                     "Reserved",
                                     "Reserved",
                                     "Reserved",
                                     "Reserved",
                                     "Reserved",
                                     "Reserved",
                                     "Reserved",
                                     "Reserved"};

void isr_handler(registers_t *reg) {
    if (interrupt_handlers[reg->int_no]) {
        isr_t handler = interrupt_handlers[reg->int_no];
        handler(reg);
    } else {
        printf("Unhandled interrupt: %d %s\n", reg->int_no, exceptions_messages[reg->int_no]);
        while (1);
    }
}

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

void irq_handler(registers_t *reg) {
    if (reg->int_no >= 40)
        outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);

    if (interrupt_handlers[reg->int_no]) {
        isr_t handler = interrupt_handlers[reg->int_no];
        handler(reg);
    }
}
