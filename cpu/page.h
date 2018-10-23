#ifndef PAGE_H
#define PAGE_H
#include<stdint.h>

uint32_t* enable_table(unsigned int);
void enable_paging();
void disable_paging();
void init_page();
void bind_page(void*, void*);
void unbind_page(void*);
#endif
