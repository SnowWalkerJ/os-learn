#include "page.h"
#include "crx.h"
#include <kernel/kmemory.h>


uint32_t *page_directory;

void init_page_directory();
void set_init_4m_paging();

void init_page() {
    init_page_directory();
	set_init_4m_paging();
	write_cr3((uint32_t)page_directory);
	write_cr0(read_cr0() | 0x80000000);
}

void bind_page(void* virtual_addr, void* physical_addr) {
    unsigned int table_id, page_id;
    table_id = (unsigned int)((uint32_t)virtual_addr >> 22);
    page_id = (unsigned int)(((uint32_t)virtual_addr >> 12) & 0x03FF);
    // offset = (uint32_t)virtual_addr & 0x03ff;
    if ((page_directory[table_id] & 1) == 0) {
        // The table_id-th table is not available yet
        enable_table(table_id);
    }
    uint32_t* table = (uint32_t*)(page_directory[table_id] ^ 0x03);
    table[page_id] = (uint32_t)physical_addr | 3;
}


void init_page_directory() {
    page_directory = (uint32_t*)kalloc_page();
	for (int i = 0; i < 1024; i++) {
		page_directory[i] = 0b10;
	}
    // make an identity map to make sure it;s accessible after paging is enabled
    bind_page((void*)page_directory, (void*)page_directory);
}

void set_init_4m_paging() {
    /* The first 4M memory is reserved for kernel.
     * So it is an identity map.
     */
	uint32_t i, address;
    uint32_t* page_table = enable_table(0);
	address = 0;
	for (i = 0; i < 1024; i++) {
		page_table[i] = address | 3;
		address += 4096;
	}
}

uint32_t* enable_table(unsigned int table_id) {
    uint32_t* table = (uint32_t*)kalloc_page();
    page_directory[table_id] = (uint32_t)table | 3;
    for (size_t i = 0; i < 1024; i++ ) {
        table[i] = 2;    // set all entries of the table as unavailable
    }
    bind_page((void*)table, (void*)table);
    return table;
}
