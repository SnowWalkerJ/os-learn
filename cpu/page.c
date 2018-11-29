#include <kernel/console.h>
#include <kernel/crx.h>
#include <kernel/kmemory.h>
#include <kernel/page.h>
#include <kernel/sched.h>
#include <libs/assert.h>
#include <stddef.h>

// uint32_t *page_directory;
static int paging_disable_level;

#define PD (current->tss.cr3)
#define PD_p ((uint32_t *)PD)
#define TABLE_ID(addr) ((unsigned int)((uint32_t)(addr) >> 22))
#define PAGE_ID(addr) ((unsigned int)(((uint32_t)(addr) >> 12) & 0x03FF))

void init_page_directory();
void set_init_4m_paging();
void enable_paging();
void disable_paging();

void init_page() {
    // assert(paging_disable_level == 1, "Paging enabled before
    // initialization");
    paging_disable_level = 1;
    init_page_directory();
    set_init_4m_paging();
    enable_paging();
}

void bind_page(void *virtual_addr, void *physical_addr) {
    unsigned int table_id, page_id;
    table_id = TABLE_ID(virtual_addr);
    page_id  = PAGE_ID(virtual_addr);
    disable_paging();
    if ((PD_p[table_id] & 1) == 0) {
        // The table_id-th table is not available yet
        enable_table(table_id);
    }
    uint32_t *table = (uint32_t *)(PD_p[table_id] & 0xFFFFF000);
    assert((table[page_id] & 0x01) == 0 || (table[page_id] & 0xfffff000) == (uint32_t)physical_addr, "binding on an existing page");
    table[page_id] = (uint32_t)physical_addr | 3;
    enable_paging();
}

void unbind_page(void *virtual_addr) {
    unsigned int table_id, page_id;
    table_id = TABLE_ID(virtual_addr);
    page_id  = PAGE_ID(virtual_addr);
    disable_paging();
    uint32_t *table = (uint32_t *)(PD_p[table_id] & 0xFFFFF000);
    if ((table[page_id] & 0x01) == 0)
        panic("unbinding a page that's not present.");
    table[page_id] = 2;
    enable_paging();
}

int page_enabled() {
    return ((uint32_t)read_cr0() & 0x80000000) != 0;
}

void init_page_directory() {
    PD = (uint32_t)kalloc_page();
    for (int i = 0; i < 1024; i++) {
        PD_p[i] = 0b10;
    }
    // make an identity map to make sure it's accessible after paging is enabled
}

void* virtual_to_physical(void *virtual_addr) {
    unsigned int table_id, page_id;
    uint32_t physical_addr;
    table_id = (unsigned int)((uint32_t)virtual_addr >> 22);
    page_id  = (unsigned int)(((uint32_t)virtual_addr >> 12) & 0x03FF);
    uint32_t *table = (uint32_t *)(PD_p[table_id] & 0xFFFFF000);
    if ((PD_p[table_id] & 0x01) == 0|| (table[page_id] & 0x01) == 0) {
        physical_addr = 0;
    } else {
        physical_addr = table[page_id] & 0xFFFFF000;
    }
    return (void *)physical_addr;
}

void set_init_4m_paging() {
    /* The first 4M memory is reserved for kernel.
     * So it is an identity map.
     */
    for (int table_id = 0; table_id < 2; table_id++) {
        uint32_t *page_table = enable_table(table_id);
        for (uint32_t i = 0; i < 1024; i++) {
            page_table[i] = (table_id << 22) | (i << 12) | 3;
        }
    }
    assert(virtual_to_physical((void *)PD) == (void *)PD, "page directory not correctly mapped");
}

uint32_t *enable_table(unsigned int table_id) {
    uint32_t *table          = (uint32_t *)kalloc_page();
    PD_p[table_id] = (uint32_t)table | 3;
    for (size_t i = 0; i < 1024; i++) {
        table[i] = 2; // set all entries of the table as unavailable
    }
    // if (!virtual_to_physical(table) && TABLE_ID(table) != table_id)
    //     bind_page(table, table);
    return table;
}

void enable_paging() {
    assert(paging_disable_level > 0, "Paging already enabled");
    assert(virtual_to_physical(PD_p) == PD_p, "Identity mapping failed to set");
    paging_disable_level--;
    if (paging_disable_level == 0) {
        write_cr3((uint32_t)PD);
        write_cr0(read_cr0() | 0x80000000);
    }
}

void disable_paging() {
    if (paging_disable_level++ == 0) {
        write_cr0(read_cr0() & ~0x80000000);
    }
}
