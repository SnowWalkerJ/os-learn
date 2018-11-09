#include <stddef.h>
#include <stdint.h>
#include <fs/ext2.h>
#include <fs/bitmap.h>


#define LBA_SIZE 512

#define SINGLY_UPNUM(block_size) (12 + block_size / 32)
#define DOUBLY_UPNUM(block_size) (12 + block_size / 32 + block_size / 32 * block_size / 32)
#define TRIPLY_UPNUM(block_size) (12 + block_size / 32 + block_size / 32 * block_size / 32 + block_size / 32 * block_size / 32 * block_size / 32)


extern void* malloc(size_t);
extern void* memcpy(void*, const void*, size_t);
extern void free(void*);

extern void read_lba(int dev, int lba, uint8_t* buffer);
extern void write_lba(int dev, int lba, uint8_t* buffer);


int initialized[2];


struct superblock superblocks[2];


struct superblock* get_super(int dev) {
    if (dev >= 2) return NULL;
    if (!initialized[dev]) {
        read_lba(0, 2, (uint8_t*)(superblocks+dev));
        read_lba(0, 3, (uint8_t*)((int)(superblocks+dev)+512));
        initialized[dev] = 1;
    }
    return superblocks+dev;
}


void read_block(int dev, uint32_t blockid, uint8_t* buffer) {
    struct superblock* sb = get_super(dev);
    int block_size = 2 << (sb->block_size + 10);
    int sectors_per_block = block_size / LBA_SIZE;
    int offset = blockid * sectors_per_block;
    for (int i = 0; i < sectors_per_block; i++) {
        read_lba(dev, offset, buffer);
        offset += 1;
        buffer += LBA_SIZE;
    }
}


void write_block(int dev, uint32_t blockid, uint8_t* buffer) {
    struct superblock* sb = get_super(dev);
    int block_size = 2 << (sb->block_size + 10);
    int sectors_per_block = block_size / LBA_SIZE;
    int offset = blockid * sectors_per_block;
    for (int i = 0; i < sectors_per_block; i++) {
        write_lba(dev, offset, buffer);
        offset += 1;
        buffer += LBA_SIZE;
    }
}


void find_inode_from_id(int dev, size_t inode_id, struct inode* result) {
    inode_id--;                                // inode id starts from 1
    struct superblock* sb = get_super(dev);
    int groups = sb->blocks / sb->blocks_per_group;
    int group = inode_id / sb->inodes_per_group;
    int block_size = 2 << (sb->block_size + 10);
    uint8_t* block_buffer = (uint8_t*)malloc(block_size);
    read_block(dev, sb->superblock + 1, block_buffer);       // Read block group descriptor table
    struct block_group_descriptor* bgd = (struct block_group_descriptor*)block_buffer;
    struct inode* inodes = (struct inode*)bgd[group].inode_table;
    memcpy(result, inodes + (inode_id % sb->inodes_per_group), sizeof(struct inode));
    free(block_buffer);
}


int read_path(char* filename) {
    int i;
    if (filename[0] == '/') filename++;
    for (i = 0; filename[i] != 0 && filename[i] != '/'; i++);
    return i; 
}


void follow_symbolic_link(uint8_t* mem, struct inode* link, struct inode* result) {
    int size = link->low_size;
    char* target_path;
    if (size < 60) {
        target_path = (char*)(&link->direct_block[0]);
    } else {
        target_path = (char*)link->direct_block[0];
    }
    find_inode_from_path(mem, target_path, result);

    // If the target is a symbolic link itself, follow it again
    if (result->type_permission >> 11 == INODE_TYPE_SYMBOLIC) {
        follow_symbolic_link(mem, result, result);
    }
}


struct _iter_dir_st {
    struct directory_entry* entry;
    size_t remain_size;
    int current_block;
};


struct block_iterator {
    uint8_t* data;
    size_t remain_size;
    int current_block;
};


struct block_iterator* __iter_block(int dev, struct inode* inode, struct block_iterator* state) {
    if (inode->hard_links == 0) return NULL;
    struct superblock* sb = get_super(dev);
    if (sb == NULL) return NULL;
    int block_size = 1<<(sb->block_size+10);

    if (state == NULL) {
        // First loop, initialize the state
        int type = inode->type_permission >> 11;
        size_t size = inode->low_size;
        if (type == INODE_TYPE_REGULAR)
            size += (size_t)inode->high_size << 16;
        state = (struct block_iterator*)malloc(sizeof(struct block_iterator));
        state->data = (uint8_t*)malloc(block_size);
        state->remain_size = size;
        state->current_block = -1;
    }

    if (state->remain_size <= 0) {
        // End of file
        return NULL;
    }

    state->current_block++;
    state->remain_size -= block_size;
    int block_id;
    if (state->current_block < 12) {
        block_id = inode->direct_block[state->current_block];
    } else if (state->current_block < SINGLY_UPNUM(block_size)) {
        
    }
    // TODO: indirect block links
    read_block(dev, block_id, state->data);
    return state;
}


void __finalize_iter_block(struct block_iterator* iter) {
    if (iter != NULL) {
        if (iter->data != NULL) {
            free(iter->data);
        }
        free(iter);
    }
}


#define iter_block(var, dev, inode, body) do{\
    struct block_iterator* var; \
    while((var = __iter_block(dev, inode, var)) != NULL) body; \
    __iter_block_finalize: \
    if (var != NULL) __finalize_iter_block(var); \
}while(0);


void test() {
    struct inode* a = NULL;
    iter_block(st, 0, a, break;);
}