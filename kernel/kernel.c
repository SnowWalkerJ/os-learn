#include <stdint.h>
#include <drivers/screen.h>
#include <libs/string.h>
#include <cpu/isr.h>
#include <cpu/page.h>
#include "memory.h"
#include "interrupt_handlers.h"
#define UNUSED(x) (void)(x)
extern void isr_install();


void init(){
	print("Kernel entered.\n");
	initMemTable();
    print("Memory initialized\n");
	isr_install();
    print("ISR installed\n");
    register_interrupt_handlers();
    print("Interrupt handlers registerred\n");
	//set_page();
	clearScreen();
}

void shell () {
	print("LearnOS> ");
}

void main () {
    init();
    shell();
}
