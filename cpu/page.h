#ifndef PAGE_H
#define PAGE_H
#include<stdint.h>
uint32_t* enable_table(unsigned int);
void init_page();
void bind_page(void*, void*);
#endif
