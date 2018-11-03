#include <libs/stdlib.h>
#include <kernel/kmemory.h>
#include <kernel/console.h>

uint32_t search_table(uint8_t*);
void set_occupied(void*, uint32_t);
void set_free(void*, uint32_t);
uint32_t get_value(void*, uint32_t);

void init_memory_table() {
    memsetb(L1_TABLE, 0, TABLE_SIZE);
    memsetb(L2_TABLE, 0, TABLE_SIZE * TABLE_SIZE * 64);
    set_occupied(L1_TABLE, 0); // reserve the first 4M for kernel
}

void* kalloc_page() {
    uint32_t id1 = search_table((uint8_t*)L1_TABLE);
    uint32_t id2 = search_table((uint8_t*)(L2_TABLE+id1*TABLE_SIZE));
    uint32_t addr = (id1 * TABLE_SIZE + id2) * PAGE_SIZE;
    set_occupied(L1_TABLE, id1);
    set_occupied(L2_TABLE+id1*TABLE_SIZE, id2);
    return (void*)addr;
}

void kfree_page(void* addr) {
    uint32_t id1, id2;
    id1 = (uint32_t)addr >> 22;
    id2 = ((uint32_t)addr >> 12) & 0x03FF;
    set_free(L2_TABLE, id2);
    if (get_value(L2_TABLE, id2) == 0) {
        set_free(L1_TABLE, id1);
    }
}

uint32_t search_table(uint8_t* table) {
    uint32_t max_id = TABLE_SIZE;
    for (uint32_t bid = 0; bid < max_id; bid++) {
        uint8_t block = table[bid];
        if (block != 0xff) {
            for (int cid = 0; cid <= 0xff; cid++) {
                if ((block & 1) == 0) {
                    return bid * 8 + cid;
                } else {
                    block >>= 1;
                }
            }
        }
    }
    return 0xffffffff;
}

uint32_t get_value(void* table, uint32_t id) {
    uint32_t high = id / (8 * sizeof(uint32_t));
    return ((uint32_t*)table)[high];
}

void set_occupied(void* table, uint32_t id) {
    uint32_t high = id / (8 * sizeof(uint32_t));
    uint32_t low = 1 << (id % (8 * sizeof(uint32_t)));
    ((uint32_t*)table)[high] |= low;
}

void set_free(void* table, uint32_t id) {
    uint32_t high = id / (8 * sizeof(uint32_t));
    uint32_t low = 1 << (id % (8 * sizeof(uint32_t)));
    ((uint32_t*)table)[high] ^= low;
}
