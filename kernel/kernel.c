#include <drivers/ata.h>
#include <drivers/screen.h>
#include <fs/buffer.h>
#include <fs/ext2.h>
#include <kernel/gdt.h>
#include <kernel/interrupt_handlers.h>
#include <kernel/isr.h>
#include <kernel/kmemory.h>
#include <kernel/memory.h>
#include <kernel/page.h>
#include <kernel/sched.h>
#include <libs/assert.h>
#include <libs/linknode.h>
#include <libs/stdio.h>
#include <libs/string.h>
#include <libs/time.h>
#include <stdint.h>
#include <tests/testings.h>
extern void isr_install();

void read_hdd() {
    struct superblock *sb = get_super(0);
    printf("Total number of blocks: %d\n", (int)sb->blocks);
    printf("Total number of inodes: %d\n", (int)sb->inodes);

    struct inode inode;
    find_inode_from_path(0, "/", &inode);
    kprint_hex(inode.type_permission >> 12);

    struct linknode *link = list_directory(0, &inode);
    int i                 = 0;
    while (link) {
        printf("%d %s\n", i, (char *)link->value);
        link = link->next;
        i++;
    }

    linknode_free(link);
}

void init() {
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
    init_block_buffers();
    kprint("Buffers initialized\n");
    run_tests();
    read_hdd();
    init_time();
    init_sched();
}

void shell() {
    clearScreen();
    kprint("\nLearnOS> ");
}

#include <fs/ext2.h>

void main() {
    init();
    shell();
}
