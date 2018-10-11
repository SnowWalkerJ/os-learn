#include "idt.h"
#include "bits.h"
#include "../kernel/memory.h"
#include "../libs/string.h"
#include "../drivers/screen.h"


void set_idt_gate(int n, uint32_t handler) {
	//char*hex = int_to_hex(n);
	//print(hex);
	//free(hex);
	//hex = int_to_hex(handler);
	//print(hex);
	//free(hex);
	idt[n].low_offset = low_16(handler);
	idt[n].sel = KERNEL_CS;
	idt[n].always0 = 0;
	idt[n].flags = 0x8E;
	idt[n].high_offset = high_16(handler);
}

void set_idt() {
	idt_reg.base = (uint32_t) &idt;
	idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;
	__asm__ __volatile__("lidtl (%0)" : : "r" (&idt_reg));
}

