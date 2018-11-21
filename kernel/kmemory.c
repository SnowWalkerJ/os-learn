#include <libs/stdlib.h>
#include <kernel/kmemory.h>
#include <kernel/console.h>
#define MAX_MEMORY_SIZE 0xFFFFFFFF
#define TABLE_SIZE 128

typedef uint8_t page_bitmap[TABLE_SIZE];
static page_bitmap l1_table = {0};
static page_bitmap l2_table[TABLE_SIZE * 8] = {{0}};

static int search_table(page_bitmap);
static void set_occupied(page_bitmap, uint16_t);
static void set_free(page_bitmap, uint16_t);
static int get_value(page_bitmap);

void init_memory_tables() {
    // memsetw(l1_table, 0, TABLE_SIZE);
    // for (int i = 0; i < TABLE_SIZE * 8; i++)
    //     memsetw(l2_table[i], 0, TABLE_SIZE / 2);
    set_occupied(l1_table, 0); // reserve the first 4M for kernel
}

void* kalloc_page() {
    int id1 = search_table(l1_table);
    if (id1 < 0) return NULL;
    int id2 = search_table(l2_table[id1]);
    if (id2 < 0) panic("inconsistent page bitmap");
    uint32_t addr = ((uint32_t)id1 * TABLE_SIZE * 8 + (uint32_t)id2) * PAGE_SIZE;

    if (get_value(l2_table[id1]) == 0xFF)
        set_occupied(l1_table, id1);
    set_occupied(l2_table[id1], id2);
    
    return (void*)addr;
}

void kfree_page(void* addr) {
    uint32_t id1, id2;
    id1 = (uint32_t)addr >> 22;
    id2 = ((uint32_t)addr >> 12) & 0x03FF;
    set_free(l2_table[id1], id2);
    if (get_value(l2_table[id1]) == 0) {
        set_free(l1_table, id1);
    }
}

static int search_table(page_bitmap table) {
    uint32_t max_id = TABLE_SIZE;
    for (uint32_t bid = 0; bid < max_id; bid++) {
        uint8_t block = table[bid];
        if (block != 0xff) {
            for (int cid = 0; cid <= 8; cid++) {
                if ((block & 1) == 0) {
                    return bid * 8 + cid;
                } else {
                    block >>= 1;
                }
            }
        }
    }
    return -1;
}

static int get_value(page_bitmap table) {
    int total = 0;
    for(int i = 0; i < TABLE_SIZE; i++) {
        total += table[i];
    }
    return total;
}

static void set_occupied(page_bitmap table, uint16_t id) {
    uint32_t high = id / (8 * sizeof(uint32_t));
    uint32_t low = 1 << (id % (8 * sizeof(uint32_t)));
    ((uint32_t*)table)[high] |= low;
}

static void set_free(page_bitmap table, uint16_t id) {
    uint32_t high = id / (8 * sizeof(uint32_t));
    uint32_t low = 1 << (id % (8 * sizeof(uint32_t)));
    ((uint32_t*)table)[high] ^= low;
}
