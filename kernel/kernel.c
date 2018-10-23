#include <stdint.h>
#include <drivers/screen.h>
#include <libs/string.h>
#include <libs/time.h>
#include <cpu/isr.h>
#include <cpu/page.h>
#include <tests/testings.h>
#include "memory.h"
#include "kmemory.h"
#include "interrupt_handlers.h"
#define UNUSED(x) (void)(x)
extern void isr_install();
void init_time();

void init(){
    clearScreen();
	kprintAt("Kernel entered.\n", 1, 0);
    init_time();
	initMemTable();
    kprint("Memory initialized\n");
	isr_install();
    kprint("ISR installed\n");
    register_interrupt_handlers();
    kprint("Interrupt handlers registerred\n");
	init_page();
    run_tests();
}

void init_time() {
    time_t tm = time();
    char* text = strftime(tm, "%Y-%m-%d %H:%M:%S");
    kprint("Boot time: ");
    kprint(text);
    kprint("\n");
    free(text);
}

void shell () {
    kprint("\nLearnOS> ");
}

void main () {
    init();
    shell();
}
