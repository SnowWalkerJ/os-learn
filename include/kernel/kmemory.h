#ifndef KMEM_H
#define KMEM_H

#define PAGE_SIZE 4096

void init_memory_tables();
void *kalloc_page();
void kfree_page(void *);

#endif
