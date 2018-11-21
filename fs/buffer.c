#include <stddef.h>
#include <fs/buffer.h>
#include <kernel/memory.h>
#include <kernel/sched.h>
#include <drivers/ata.h>


#define NR_BUFFER 128
#define NR_HASHTABLE 307
#define _hashfn(dev, block) (((dev)^(block)) % NR_HASHTABLE)
#define hash(dev, block) hashtable[_hashfn(dev, block)]


static struct buffer_head* free_buffers;
static struct buffer_head* hashtable[NR_HASHTABLE];
static struct task_struct * buffer_wait = NULL;


static inline void wait_on_buffer(struct buffer_head*);
static inline void remove_from_queues(struct buffer_head* bh);
static inline void insert_into_queues(struct buffer_head* bh);
static inline void sync_dev(int);


static struct buffer_head* find_block_in_hashtable(int dev, int block) {
    struct buffer_head* bh;
    for (bh = hash(dev, block); bh != NULL; bh = bh->next) {
        if (bh->dev == dev && bh->block == block) {
            return bh;
        }
    }
    return NULL;
}


static struct buffer_head* get_hash_table(int dev, int block) {
    struct buffer_head* bh;
    while (1) {
        if (!(bh = find_block_in_hashtable(dev, block))) {
            return NULL;
        }
        bh->count++;
        wait_on_buffer(bh);
        if (bh->dev == dev && bh->block == block) {
            return bh;
        }
        bh->count--;
    }
}


#define BADNESS(bh) (((bh)->dirty<<1) + (bh)->locked)


struct buffer_head* getblk (int dev, int block) {
    struct buffer_head *bh, *tmp;

    repeat:
    if ((bh = get_hash_table(dev, block)))
        return bh;

    tmp = free_buffers;
    do {
        if (tmp->count) continue;
        if (!bh || BADNESS(tmp) < BADNESS(bh)) {
            bh = tmp;
            if (!BADNESS(bh)) break;
        }
    } while ((tmp = tmp->next_free) != free_buffers);

    if (!bh) {
        sleep_on(&buffer_wait);
        goto repeat;
    }

    wait_on_buffer(bh);

    if (bh->count) goto repeat;

    while (bh->dirty) {
        sync_dev(bh->dev);
        wait_on_buffer(bh);
        if (bh->count)
            goto repeat;
    }

    if (find_block_in_hashtable(dev, block))
        goto repeat;
    
    bh->count = 1;
    bh->dirty = 0;
    bh->uptodate = 0;
    bh->locked = 0;
    remove_from_queues(bh);
    bh->dev = dev;
    bh->block = block;
    insert_into_queues(bh);
    return bh;
}


void rlsblk (struct buffer_head* bh) {
    if (!bh) return;
    wait_on_buffer(bh);
    if ((bh->count--) == 0)
        panic("Freeing a free buffer");
    wake_up(&buffer_wait);
}


void init_block_buffers () {
    struct buffer_head *last_bh = NULL, *bh;
    free_buffers = NULL;
    for (int i = 0; i < NR_BUFFER; i++) {
        bh = malloc(sizeof(struct buffer_head));
        if (!free_buffers) {
            free_buffers = last_bh = bh;
        }
        bh->block = 0;
        bh->count = 0;
        bh->data = malloc(4096);
        bh->dev = 0;
        bh->dirty = 0;
        bh->locked = 0;
        bh->next = NULL;
        last_bh->next_free = bh;
        bh->prev_free = last_bh;
        bh->next_free = free_buffers;
        bh->uptodate = 0;
        last_bh = bh;
    }
    for (int i = 0; i < NR_HASHTABLE; i++) {
        hashtable[i] = NULL;
    }
}


static inline void remove_from_queues(struct buffer_head* bh) {
    if (bh->prev)
        bh->prev->next = bh->next;
    if (bh->next)
        bh->next->prev = bh->prev;
    
    if (hash(bh->dev, bh->block) == bh) hash(bh->dev, bh->block) = bh->next;

    if (!bh->next_free || !bh->prev_free) {
        panic("Corrupted buffer list");
    }

    bh->prev_free->next_free = bh->next_free;
    bh->next_free->prev_free = bh->prev_free;
    if (free_buffers == bh) free_buffers = bh->next_free;
}


static inline void insert_into_queues(struct buffer_head* bh) {
    if (free_buffers == NULL) {
        free_buffers = bh;
        bh->prev_free = bh->next_free = bh;
    } else {
        free_buffers->prev_free->next_free = bh;
        bh->prev_free = free_buffers->prev_free;
        bh->next_free = free_buffers;
        free_buffers->prev_free = bh;
    }

    bh->next = bh->prev = NULL;
    if (bh->dev < 0) return;
    bh->next = hash(bh->dev, bh->block);
    hash(bh->dev, bh->block) = bh;
    if (bh->next != NULL)
        bh->next->prev = bh;
}


struct buffer_head* bread(int dev, int block) {
    struct buffer_head* bh;
    if (!(bh = getblk(dev, block))) 
        panic("bread: return NULL from getblk");
    if (bh->uptodate)
        return bh;
    uint32_t sector = bh->block * BUFFER_SIZE / SECTOR_SIZE;
    bh->locked = 1;
    for (uint32_t i = 0; i < BUFFER_SIZE / SECTOR_SIZE; i++){
        pio_read_lba(bh->dev, sector+i, bh->data + i * SECTOR_SIZE);
    }
    wait_on_buffer(bh);
    bh->uptodate = 1;
    if (bh->uptodate)
        return bh;
    rlsblk(bh);
    return NULL;
}


static inline void wait_on_buffer(struct buffer_head* bh) {
    while (bh->locked) {
        asm volatile("nop;nop;nop;nop;"
                     "nop;nop;nop;nop;"
                     "nop;nop;nop;nop;");
    }
}


static inline void sync_dev(int dev) {
    (void)(dev);
}
