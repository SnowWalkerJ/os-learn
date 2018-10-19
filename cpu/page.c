#include "page.h"
#include "crx.h"


uint32_t *page_directory = (uint32_t*)0x9C000;
uint32_t *page_table = (uint32_t*)0x9D000;


void set_init_4m_paging() {
	uint32_t i, address;
	address = 0;
	for (i = 0; i < 1024; i++) {
		page_table[i] = address | 3;
		address += 4096;
	}
	page_directory[0] = (uint32_t)page_table | 3;
	for (i = 1; i < 1024; i++) {
		page_directory[i] = 0b10;
	}
}

void set_page() {
	set_init_4m_paging();
	write_cr3((uint32_t)page_directory);
	write_cr0(read_cr0() | 0x80000000);
}
