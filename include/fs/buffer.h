#ifndef BUFFER_H
#define BUFFER_H
#include <stdint.h>


struct buffer_head {
    int dev;
    int block;
    uint8_t* data;
    int uptodate;
    int dirty;
    int count;
    int locked;
    struct buffer_head* next;
    struct buffer_head* prev;
    struct buffer_head* next_free;
    struct buffer_head* prev_free;
};


void init_block_buffers();
struct buffer_head* getblk(int, int);
void rlsblk(struct buffer_head*);

#endif /* BUFFER_H */