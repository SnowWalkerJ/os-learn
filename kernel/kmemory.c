#include <libs/stdlib.h>
#include <libs/bitmap.h>
#include <kernel/kmemory.h>
#include <kernel/console.h>
#define MAX_MEMORY_SIZE 0xFFFFFFFF
#define TABLE_SIZE 128

typedef uint8_t page_bitmap[TABLE_SIZE];
static page_bitmap l1_table = {0};
static page_bitmap l2_table[TABLE_SIZE * 8] = {{0}};

static unsigned int search_table(page_bitmap);
static void set_occupied(page_bitmap, uint16_t);
static void set_free(page_bitmap, uint16_t);
static int get_value(page_bitmap);

void init_memory_tables() {
    set_occupied(l1_table, 0); // reserve the first 4M for kernel
}

void* kalloc_page() {
    int id1 = search_table(l1_table);
    if (id1 >= TABLE_SIZE*8) return NULL;
    int id2 = search_table(l2_table[id1]);
    if (id2 >= TABLE_SIZE*8) panic("inconsistent page bitmap");
    uint32_t addr = ((uint32_t)id1 * TABLE_SIZE * 8 + (uint32_t)id2) * PAGE_SIZE;

    if (get_value(l2_table[id1]) == TABLE_SIZE * 8)
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

static unsigned int search_table(page_bitmap table) {
    struct bitmap bm = {TABLE_SIZE, table};
    return first_zero(&bm);
}

static inline int bit_count(uint32_t n) {
    int count;
    for (count = 0; n; count++) {
        n &= n-1;
    }
    return count;
}


static int get_value(page_bitmap table) {
    int total = 0;
    for (int i = 0; i < (int)(TABLE_SIZE / sizeof(uint32_t)); i++) {
        total += bit_count(((uint32_t*)table)[i]);
    }
    return total;
}

static void set_occupied(page_bitmap table, uint16_t id) {
    struct bitmap bm = {TABLE_SIZE, table};
    set_one(&bm, id);
}

static void set_free(page_bitmap table, uint16_t id) {
    struct bitmap bm = {TABLE_SIZE, table};
    set_zero(&bm, id);
}
