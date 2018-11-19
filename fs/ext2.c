#include <stddef.h>
#include <stdint.h>
#include <fs/ext2.h>
#include <fs/bitmap.h>
#include <libs/string.h>


#define LBA_SIZE 512

#define BLOCK_SIZE(sb) (1 << ((sb)->block_size + 10))

#define SINGLY_UPNUM(block_size) (12 + block_size / 32)
#define DOUBLY_UPNUM(block_size) (12 + block_size / 32 + block_size / 32 * block_size / 32)
#define TRIPLY_UPNUM(block_size) (12 + block_size / 32 + block_size / 32 * block_size / 32 + block_size / 32 * block_size / 32 * block_size / 32)


extern void* malloc(size_t);
extern void* memcpy(void*, const void*, size_t);
extern void free(void*);

extern void read_lba(int dev, int lba, uint8_t* buffer);
extern void write_lba(int dev, int lba, uint8_t* buffer);


int initialized[4];


struct superblock superblocks[4];


struct superblock* get_super(int dev) {
    if (dev >= 4) return NULL;
    if (!initialized[dev]) {
        read_lba(0, 2, (uint8_t*)(superblocks+dev));
        read_lba(0, 3, (uint8_t*)((int)(superblocks+dev)+512));
        initialized[dev] = 1;
    }
    return superblocks+dev;
}


void read_block(int dev, uint32_t blockid, uint8_t* buffer) {
    struct superblock* sb = get_super(dev);
    int block_size = 1 << (sb->block_size + 10);
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
    int block_size = 1 << (sb->block_size + 10);
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
    int group = inode_id / sb->inodes_per_group;
    int block_size = 1 << (sb->block_size + 10);
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


void follow_symbolic_link(int dev, struct inode* link, struct inode* result) {
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
    if (filename[0] != '/') {
        printf("Must enter an absolute path\n");
        return -1;
    }
    char *name = filename + 1;
    int length;
    char* part_name;
    find_inode_from_id(dev, 2, result);          // Root inode
    while(filename[length = read_path(name)] != 0) {
        if (result->hard_links == 0) return NULL;               // File already deleted
        int type = result->type_permission >> 11;
        switch (type) {
        case INODE_TYPE_DIRECTORY:
            part_name = (char*)malloc((length+1) * sizeof(char));
            memcpy(part_name, name, length);
            part_name[length] = 0;
            if (search_directory(dev, result, part_name, result) != 0) {                               // Can't find the file in the directory
                return -3;
            }
            free(part_name);
            break;
        case INODE_TYPE_SYMBOLIC:
            follow_symbolic_link(dev, result, result);
            break;
        default:
            printf("Unable to find inode of %s\n", filename);
            return -2;
        }
        name += length;
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
        memcpy(inode->direct_block, state->lv0_table, sizeof(inode->direct_block));
    }

    if (state->remain_size <= 0) {
        // End of file
        return NULL;
    }

    state->lv0++;
    if (state->lv0 >= state->lv0_size) {
        state->lv0 = 0;
        state->lv0_size = block_size / sizeof(uint32_t);
        state->lv1++;
        if (state->lv1 >= block_size / sizeof(uint32_t)) {
            state->lv1 = 0;
            state->lv2++;
            if (state->lv2 == 0)
                read_block(dev, inode->doubly_indirect_block, state->lv2_table);
            read_block(dev, state->lv2_table[state->lv2], state->lv1_table);
        }
        read_block(dev, state->lv1_table[state->lv1], state->lv0_table);
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


int search_directory(int dev, struct inode* dir, char* filename, struct inode* result) {
    int total_size = dir->low_size;
    struct directory_entry* entry;
    struct block_iterator* iter;
    struct superblock* sb = get_super(dev);
    iter_block(iter, dev, dir, ({
        entry = (struct directory_entry*)iter->data;
        do {
            if (strncmp(entry->name, filename, entry->name_length) == 0) {
                goto end;
            } else {
                entry = (struct directory_entry*)((uint8_t*)entry + entry->size);
            }
        } while ((void*)entry < (void*)(iter->data + BLOCK_SIZE(sb)));
        continue;
        end:
            break;
    }));
    if (strncmp(entry->name, filename, entry->name_length) == 0) {
        find_inode_from_id(dev, entry->inode, result);
        return 0;
    } else {
        return -1;
    }
}