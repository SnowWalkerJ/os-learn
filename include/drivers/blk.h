#ifndef BLK_H
#define BLK_H
#include <fs/buffer.h>

void ll_rw_block(int, struct buffer_head*);

#endif /* BLK_H */