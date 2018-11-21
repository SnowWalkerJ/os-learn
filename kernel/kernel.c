#include <stdint.h>
#include <drivers/screen.h>
#include <drivers/ata.h>
#include <libs/string.h>
#include <libs/time.h>
#include <libs/stdio.h>
#include <kernel/isr.h>
#include <kernel/page.h>
#include <kernel/gdt.h>
#include <tests/testings.h>
#include <kernel/memory.h>
#include <kernel/kmemory.h>
#include <kernel/interrupt_handlers.h>
#include <libs/assert.h>
extern void isr_install();
void init_time();

void init(){
    set_style(WHITE_ON_BLACK);
    clearScreen();
	kprintAt("Kernel entered.\n", 1, 0);
    init_gdt();
    kprint("Kernel take control of gdt\n");
    init_memory_tables();
    init_page();
    init_malloc();
    kprint("Memory initialized\n");
    isr_install();
    kprint("ISR installed\n");
    register_interrupt_handlers();
    kprint("Interrupt handlers registered\n");
    init_hdd();
    kprint("Hard drives initialized\n");
    run_tests();
    // init_time();
}

void init_time() {
    time_t tm;
    time(&tm);
    char* text = (char*)malloc(20);
    strftime(&tm, "%Y-%m-%d %H:%M:%S", text);
    printf("Boot time: %s UTC\n", text);
    free(text);
}

void shell () {
    kprint("\nLearnOS> ");
}

void main () {
    init();

    char* data = (char*)malloc(SECTOR_SIZE);
    pio_read_lba(0, 0, data);
    data[5] = 0;
    printf("%s", data);
    free(data);

    shell();
}
