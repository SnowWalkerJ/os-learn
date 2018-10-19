#include <stdint.h>
#include <drivers/screen.h>
#include <libs/string.h>
#include <cpu/isr.h>
#include <cpu/page.h>
#include "memory.h"
#include "kmemory.h"
#include "interrupt_handlers.h"
#define UNUSED(x) (void)(x)
extern void isr_install();


void init(){
	kprintAt("Kernel entered.\n", 1, 0);
	initMemTable();
    kprint("Memory initialized\n");
	isr_install();
    kprint("ISR installed\n");
    register_interrupt_handlers();
    kprint("Interrupt handlers registerred\n");
	init_page();
	clearScreen();
}

void shell () {
	kprint("LearnOS> ");
}

void main () {
    init();
    shell();
}
