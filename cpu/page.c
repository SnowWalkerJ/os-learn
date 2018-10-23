#include "page.h"
#include "crx.h"
#include <kernel/console.h>
#include <kernel/kmemory.h>
#include <libs/assert.h>

/* TODO: disable paging if a new page table is not accessible */

uint32_t *page_directory;
int paging_disable_level;

void init_page_directory();
void set_init_4m_paging();
void enable_paging();
void disable_paging();

void init_page() {
    // assert(paging_disable_level == 1, "Paging enabled before initialization");
    paging_disable_level = 1;
    init_page_directory();
	set_init_4m_paging();
    enable_paging();
}

void bind_page(void* virtual_addr, void* physical_addr) {
    unsigned int table_id, page_id;
    table_id = (unsigned int)((uint32_t)virtual_addr >> 22);
    page_id = (unsigned int)(((uint32_t)virtual_addr >> 12) & 0x03FF);
    disable_paging();
    if ((page_directory[table_id] & 1) == 0) {
        // The table_id-th table is not available yet
        enable_table(table_id);
    }
    uint32_t* table = (uint32_t*)(page_directory[table_id] & (~(uint32_t)3));
    assert((table[page_id] & 0x01) == 0, "binding on an existing page"); 
    table[page_id] = (uint32_t)physical_addr | 3;
    enable_paging();
}

void unbind_page(void* virtual_addr) {
    unsigned int table_id, page_id;
    table_id = (unsigned int)((uint32_t)virtual_addr >> 22);
    page_id = (unsigned int)(((uint32_t)virtual_addr >> 12) & 0x03FF);
    disable_paging();
    uint32_t* table = (uint32_t*)(page_directory[table_id] & (~(uint32_t)3));
    if ((table[page_id] & 0x01) == 0) return;
    table[page_id] = 2;
    enable_paging();
}


void init_page_directory() {
    page_directory = (uint32_t*)kalloc_page();
	for (int i = 0; i < 1024; i++) {
		page_directory[i] = 0b10;
	}
    // make an identity map to make sure it;s accessible after paging is enabled
}

void set_init_4m_paging() {
    /* The first 4M memory is reserved for kernel.
     * So it is an identity map.
     */
	uint32_t i;
    uint32_t* page_table = enable_table(0);
	for (i = 0; i < 1024; i++) {
		page_table[i] = (i<<12) | 3;
	}
}

uint32_t* enable_table(unsigned int table_id) {
    uint32_t* table = (uint32_t*)kalloc_page();
    page_directory[table_id] = (uint32_t)table | 3;
    for (size_t i = 0; i < 1024; i++ ) {
        table[i] = 2;    // set all entries of the table as unavailable
    }
    return table;
}

void enable_paging() {
    assert(paging_disable_level > 0, "Paging already enabled");
    paging_disable_level--;
    if (paging_disable_level == 0) {
	    write_cr3((uint32_t)page_directory);
	    write_cr0(read_cr0() | 0x80000000);
    }
}

void disable_paging () {
    if (paging_disable_level++ == 0) {
        write_cr0(read_cr0() ^ 0x80000000);
    }
}
