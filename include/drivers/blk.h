#ifndef BLK_H
#define BLK_H
#include <fs/buffer.h>

void read_block(struct buffer_head*, int);

#endif /* BLK_H */