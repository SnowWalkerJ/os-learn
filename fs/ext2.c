#include <stddef.h>
#include <stdint.h>
#include <fs/ext2.h>
#include <fs/bitmap.h>
#include <fs/buffer.h>
#include <drivers/ata.h>
#include <libs/string.h>
#include <libs/stdlib.h>
#include <libs/linknode.h>
#include <kernel/memory.h>


#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif /* MIN */

#define BLOCK_SIZE(sb) (1 << ((sb)->block_size + 10))


int search_directory(int dev, struct inode* dir, char* filename, int filename_length, struct inode* result);
int find_inode_from_path(int dev, char* filename, struct inode* result);
void follow_symbolic_link(int dev, struct inode* link, struct inode* result);
void find_inode_from_id(int dev, size_t inode_id, struct inode* result);


static int initialized[4];


static struct superblock superblocks[4];


struct superblock* get_super(int dev) {
    if (dev >= 4) return NULL;
    if (!initialized[dev]) {
        struct buffer_head* bh = bread(dev, 1);
        memcpy(superblocks+dev, bh->data, sizeof(struct superblock));
        rlsblk(bh);
        initialized[dev] = 1;
    }
    return superblocks+dev;
}


void read_block(int dev, uint32_t blockid, uint8_t* buffer) {
    struct superblock* sb = get_super(dev);
    int sectors_per_block = BLOCK_SIZE(sb) / BUFFER_SIZE;
    int offset = blockid * sectors_per_block;
    for (int i = 0; i < sectors_per_block; i++) {
        struct buffer_head* bh = bread(dev, offset++);
        memcpy(buffer, bh->data, BUFFER_SIZE);
        buffer += BUFFER_SIZE;
        rlsblk(bh);
    }
}


void find_inode_from_id(int dev, size_t inode_id, struct inode* result) {
    inode_id--;                                // inode id starts from 1
    struct superblock* sb = get_super(dev);
    int group = inode_id / sb->inodes_per_group;
    uint8_t* block_buffer = (uint8_t*)malloc(BLOCK_SIZE(sb));
    read_block(dev, sb->superblock + 1, block_buffer);       // Read block group descriptor table
    struct block_group_descriptor* bgd = (struct block_group_descriptor*)block_buffer;
    uint32_t inode_table_blkid = bgd[group].inode_table;
    read_block(dev, inode_table_blkid, block_buffer);
    memcpy(result, (struct inode*)block_buffer + (inode_id % sb->inodes_per_group), sizeof(struct inode));
    free(block_buffer);
}


static inline int read_path(char* filename) {
    int i;
    for (i = 0; filename[i] != 0 && filename[i] != '/'; i++);
    return i; 
}


void follow_symbolic_link(int dev, struct inode* link, struct inode* result) {
    /*
    Find the target inode the symlink links to.
    */
    int size = link->low_size;
    char* target_path;
    if (size < 60) {
        target_path = (char*)(&link->direct_block[0]);
    } else {
        target_path = (char*)link->direct_block[0];
    }
    find_inode_from_path(dev, target_path, result);

    // If the target is a symbolic link itself, follow it again
    if (result->type_permission >> 11 == INODE_TYPE_SYMBOLIC) {
        follow_symbolic_link(dev, result, result);
    }
}


int find_inode_from_path(int dev, char* filename, struct inode* result) {
    // TODO: support relative path
    if (filename[0] != '/') {
        // Must enter an absolute path
        return -1;
    }

    char *name = filename + 1;                   // lstrip '/'
    int length;
    find_inode_from_id(dev, 2, result);          // Root inode

    while((length = read_path(name)) != 0) {
        if (result->hard_links == 0) return -1;               // File already deleted
        int type = result->type_permission >> 11;
        switch (type) {
        case INODE_TYPE_DIRECTORY:
            if (search_directory(dev, result, name, length, result) != 0) {                               // Can't find the file in the directory
                return -3;
            }
            break;
        case INODE_TYPE_SYMBOLIC:
            follow_symbolic_link(dev, result, result);
            break;
        default:
            // Unable to find inode
            return -2;
        }
        name += length;
        if(name[0] == '/') name++;
    }
    return 0;
}


struct _iter_dir_st {
    struct directory_entry* entry;
    size_t remain_size;
    int current_block;
};


struct block_iterator {
    uint8_t* data;
    size_t remain_size;
    int lv0;
    int lv1;
    int lv2;
    int lv0_size;
    uint32_t* lv0_table;
    uint32_t* lv1_table;
    uint32_t* lv2_table;
};


struct block_iterator* __iter_block(int dev, struct inode* inode, struct block_iterator* state) {
    if (inode->hard_links == 0) return NULL;
    struct superblock* sb = get_super(dev);
    if (sb == NULL) return NULL;
    int block_size = 1 << (sb->block_size + 10);

    if (state == NULL) {
        // First loop, initialize the state
        int type = inode->type_permission >> 11;
        size_t size = inode->low_size;
        if (type == INODE_TYPE_REGULAR)
            size += (size_t)inode->high_size << 16;
        state = (struct block_iterator*)malloc(sizeof(struct block_iterator));
        state->data = (uint8_t*)malloc(block_size);
        state->remain_size = size;
        state->lv0 = state->lv1 = state->lv2 = -1;
        state->lv0_size = 12;
        state->lv0_table = (uint32_t*)malloc(block_size);
        state->lv1_table = (uint32_t*)malloc(block_size);
        state->lv2_table = (uint32_t*)malloc(block_size);
        memcpy(state->lv0_table, inode->direct_block, sizeof(inode->direct_block));
    }

    if (state->remain_size <= 0) {
        // End of file
        return NULL;
    }

    state->lv0++;
    if (state->lv0 >= state->lv0_size) {
        state->lv0 = 0;
        state->lv0_size = (int)(block_size / sizeof(uint32_t));
        state->lv1++;
        if (state->lv1 >= (int)(block_size / sizeof(uint32_t))) {
            state->lv1 = 0;
            state->lv2++;
            if (state->lv2 == 0)
                read_block(dev, inode->doubly_indirect_block, (void*)state->lv2_table);
            read_block(dev, state->lv2_table[state->lv2], (void*)state->lv1_table);
        }
        read_block(dev, state->lv1_table[state->lv1], (void*)state->lv0_table);
    }

    state->remain_size -= block_size;
    int block_id = state->lv0_table[state->lv0];
    read_block(dev, block_id, state->data);
    return state;
}


void __finalize_iter_block(struct block_iterator* iter) {
    if (iter != NULL) {
        if (iter->data != NULL) {
            free(iter->data);
        }
        free(iter->lv2_table);
        free(iter->lv1_table);
        free(iter->lv0_table);
        free(iter);
    }
}


#define iter_block(var, dev, inode, body) do{ \
    while((var = __iter_block(dev, inode, var)) != NULL) body; \
    if (var != NULL) __finalize_iter_block(var); \
}while(0);


int search_directory(int dev, struct inode* dir, char* filename, int filename_length, struct inode* result) {
    struct directory_entry* entry;
    struct block_iterator* iter;
    struct superblock* sb = get_super(dev);
    iter_block(iter, dev, dir, ({
        entry = (struct directory_entry*)iter->data;
        do {
            if (strncmp(entry->name, filename, filename_length) == 0) {
                goto end;
            } else {
                entry = (struct directory_entry*)((uint8_t*)entry + entry->size);
            }
        } while ((void*)entry < (void*)(iter->data + MIN(iter->remain_size, (size_t)BLOCK_SIZE(sb))));
        continue;
        end:
            break;
    }));
    if (strncmp(entry->name, filename, filename_length) == 0) {
        find_inode_from_id(dev, entry->inode, result);
        return 0;
    } else {
        return -1;
    }
}


struct linknode* list_directory(int dev, struct inode* dir) {
    struct linknode root = {NULL, NULL};
    struct directory_entry* entry;
    struct block_iterator* iter;
    struct superblock* sb = get_super(dev);
    iter_block(iter, dev, dir, ({
        entry = (struct directory_entry*)iter->data;
        do {
            char* filename = (char*)malloc(entry->name_length+1);
            memcpy(filename, entry->name, entry->name_length+1);
            linknode_append(&root, filename);
            entry = (struct directory_entry*)((uint8_t*)entry + entry->size);
        } while ((void*)entry < (void*)(iter->data + MIN(iter->remain_size, (size_t)BLOCK_SIZE(sb))));
    }));
    return root.next;
}
