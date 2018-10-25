#include <stdint.h>
#include <drivers/screen.h>
#include <libs/string.h>
#include <libs/time.h>
#include <libs/stdio.h>
#include <cpu/isr.h>
#include <cpu/page.h>
#include <cpu/gdt.h>
#include <tests/testings.h>
#include "memory.h"
#include "kmemory.h"
#include "interrupt_handlers.h"
#define UNUSED(x) (void)(x)
extern void isr_install();
void init_time();

void init(){
    init_gdt();
    clearScreen();
	kprintAt("Kernel entered.\n", 1, 0);
	initMemTable();
    kprint("Memory initialized\n");
	isr_install();
    kprint("ISR installed\n");
    register_interrupt_handlers();
    kprint("Interrupt handlers registered\n");
	init_page();
    run_tests();
    init_time();
}

void init_time() {
    time_t tm = time();
    char* text = strftime(tm, "%Y-%m-%d %H:%M:%S");
    printf("Boot time: %s UTC\n", text);
    free(text);
}

void shell () {
    kprint("\nLearnOS> ");
}

void main () {
    init();
    shell();
}
