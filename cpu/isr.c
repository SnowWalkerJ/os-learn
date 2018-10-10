#include<stddef.h>
#include "isr.h"
#include "idt.h"
#include "../drivers/screen.h"
#include "../kernel/memory.h"
#include "../libs/string.h"
#include "port.h"

#define PIC1_CMD 0x20
#define PIC2_CMD 0xA0
#define PIC1_DATA (PIC1_CMD+1)
#define PIC2_DATA (PIC2_CMD+1)
#define PIC_EOI 0x20

isr_t interrupt_handlers[256];

char* exceptions_messages[] = {
	"Division By Zero",
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
	"Reserved"
};

void isr_handler(registers_t reg) {
	return;
	print("received interrupt: ");
	char*hex = int_to_hex(reg.int_no);
	print(hex);
	print("\n");
	free(hex);
	print(exceptions_messages[reg.int_no]);
	print("\n");
}

void register_interrupt_handler(uint8_t n, isr_t handler) {
	char* hex = int_to_hex((uint32_t)handler);
	print(hex);
	free(hex);
	hex = int_to_hex((uint32_t)n);
	print(hex);
	free(hex);
	interrupt_handlers[n] = handler;
}

void irq_handler(registers_t reg) {
	return;
	if (reg.int_no >= 40) portByteOut(PIC2_CMD, PIC_EOI);
	portByteOut(PIC1_CMD, PIC_EOI);

	if (interrupt_handlers[reg.int_no]) {
		isr_t handler = interrupt_handlers[reg.int_no];
		handler(reg);
	}
}
