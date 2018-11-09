#ifndef KMEM_H
#define KMEM_H
#include<stdint.h>
#include<stddef.h>
#define MAX_MEMORY_SIZE 0xFFFFFFFF
#define PAGE_SIZE 4096
#define L1_TABLE (void*)0x10000
#define L2_TABLE (void*)0x10010
#define TABLE_SIZE 128

void init_memory_tables();
void* kalloc_page();
void kfree_page(void*);

#endif
