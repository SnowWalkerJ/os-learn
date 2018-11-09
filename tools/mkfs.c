#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fs/ext2.h>
#include <fs/bitmap.h>

#define max(a, b) ((a) > (b) ? (a) : (b))
#define SINGLY_UPNUM(sb) (12 + (sb)->block_size / 32)
#define DOUBLY_UPNUM(sb) (12 + (sb)->block_size / 32 + (sb)->block_size / 32 * (sb)->block_size / 32)
#define TRIPLY_UPNUM(sb) (12 + (sb)->block_size / 32 + (sb)->block_size / 32 * (sb)->block_size / 32 * (sb)->block_size / 32)


uint8_t* buffer;


void init_superblock(uint8_t*, int, int);
void init_block_descriptors(uint8_t*);
void init_block_groups(uint8_t*);
void create_root_inode(uint8_t*);
struct inode* find_inode_from_path(uint8_t*, char*);
struct inode* find_inode_from_id(uint8_t*, size_t);
void read_block(uint8_t*, uint32_t);
int log2_(int);


void mkfs(char* filename, int size, int blocksize) {
    uint8_t *mem = (uint8_t*)malloc(size);
    buffer = (uint8_t*)malloc(blocksize);
    init_superblock(mem, size, blocksize);
    init_block_descriptors(mem);
    init_block_groups(mem);

    int fp = open(filename, O_WRONLY | O_CREAT);
    if (fp == -1) {
        printf("Fail to open file\n");
        exit(-1);
    }
    write(fp, mem, size);
    close(fp);
    free(mem);
    free(buffer);
}

void init_superblock(uint8_t* mem, int size, int blocksize) {
    struct superblock sb;
    unsigned int blocks = size / blocksize;
    unsigned int blocks_per_group = blocksize * 8;
    unsigned int inodes_per_group = blocks_per_group;
    unsigned int groups = blocks / blocks_per_group;
    if (groups * 32 > blocksize) {
        printf("The size is too big for the blocksize\n");
        exit(-1);
    }
    time_t tm = time(NULL);
    sb.blocks = blocks;
    sb.inodes = blocks;
    sb.sublocks = 0;
    int inode_table_size = inode_table_size / blocksize + (inode_table_size % blocksize > 0);;
    // each group costs a descriptor, an inode bitmap, a group bitmap and an inode table
    sb.free_blocks = blocks - (3 + inode_table_size) * groups;
    if (blocksize == 1024) {
        sb.free_blocks -= 2;                // MBR + superblock
    } else {
        sb.free_blocks -= 1;                // for blocksize > 1024, MBR and superblock take the same block
    }
    sb.free_inodes = sb.inodes - 1;         // Only root inode is taken
    sb.superblock = blocksize == 1024 ? 1 : 0;
    sb.block_size = log2_(blocksize) - 10;
    sb.fragment_size = log2_(blocksize) - 10;     // This is of no use
    sb.blocks_per_group = blocks_per_group;
    sb.fragments_per_group = blocks_per_group;
    sb.inodes_per_group = inodes_per_group;
    sb.mount_time = tm;
    sb.write_time = tm;
    sb.num_mounts = 0;
    sb.num_sc_fsck = 0;
    sb.magic = 0xEF53;
    sb.state = FS_STATE_CLEAN;
    sb.errors = ERR_PANIC;
    sb.minor_version = 0;
    sb.fsck_interval = 10;
    sb.fsck_time = tm;
    sb.creator = CREATOR_LINUX;
    sb.major_version = 1;
    sb.uid = 0;
    sb.gid = 0;
    sb.first_inode = 1;
    sb.inode_size = 128;
    sb.group = 0;
    sb.opt_features = OPT_FEATURE_PREALLOC;
    sb.rqd_features = RQD_FEATURE_TYPE;
    sb.rdo_features = 0;
    memcpy(sb.fsid, "a6389b6d5ee9f015", 16);
    memcpy(sb.volume_name, "volume", 7);
    memcpy(sb.mnt_path, "/", 2);
    sb.compr_alg = 0;
    sb.prealloc_blocks_dirs = 1;
    sb.prealloc_blocks_files = 1;

    memcpy(mem+1024, &sb, sizeof(struct superblock));
}

void init_block_descriptors(uint8_t *mem) {
    struct superblock* sb = (struct superblock*)(mem+1024);
    int block_size = 2 << (sb->block_size + 10);
    //if blocksize = 1024, then block id starts from 1
    int addition = block_size == 1024 ? 1 : 0;
    int groups = sb->blocks / sb->blocks_per_group;
    int offset = max(block_size, 2048);
    for (int grp = 0; grp < groups; grp++) {
        struct block_group_descriptor bgd = {
            .block_bitmap = grp * sb->blocks_per_group + addition,
            .inode_bitmap = grp * sb->blocks_per_group + addition + 1,
            .inode_table = grp * sb->blocks_per_group + addition + 2,
            .num_free_blocks = sb->blocks_per_group - 2 - sb->inodes_per_group * sizeof(struct inode) / block_size,
            .num_free_inodes = sb->inodes_per_group,
            .num_dirs = 0
        };
        memcpy(mem+offset+grp*sizeof(struct block_group_descriptor), &bgd, sizeof(struct block_group_descriptor));
    }
}


void init_block_groups(uint8_t* mem) {
    struct superblock* sb = (struct superblock*)(mem+1024);
    int offset, reserved = 2;
    int block_size = 1 << (sb->block_size + 10);
    // printf("%d %d %d\n", sb->inodes_per_group, sizeof(struct inode), block_size);
    unsigned int inode_table_size = sb->inodes_per_group * sizeof(struct inode) / block_size;
    int groups = sb->blocks / sb->blocks_per_group;
    if (block_size == 1024) {
        offset = 1;
    } else {
        offset = 0;
    }
    for (int grp = 0; grp < groups; grp++) {
        unsigned long start = offset + reserved;
        memset((char*)start, 0, block_size*2+inode_table_size);  // set two blocks of bitmap to zero
        
        // set block bitmap
        struct bitmap bm = {
            .size=block_size,
            .map=(char*)start
        };
        int inode_table_blocks = inode_table_size / block_size + (inode_table_size % block_size > 0);
        for (int i = 0; i < reserved + 2 + inode_table_blocks; i++) {
            set_one(&bm, i);
        }
        offset += block_size;
        reserved = 0;
    }
}


void create_root_inode(uint8_t* mem) {
    struct superblock* sb = (struct superblock*)(mem+1024);
    struct inode* root = find_inode_from_id(mem, 2);                    // Root inode is always of id 2
    time_t tm = time(NULL);
    root->type_permission = INODE_TYPE_DIRECTORY << 11 | INODE_GROUP_READ | INODE_USER_READ | INODE_USER_WRITE | INODE_OTHER_READ;
    root->uid = 0;
    root->low_size = 0;
    root->atime = tm;
    root->ctime = tm;
    root->mtime = tm;
    root->dtime = 0;
    root->gid = 0;
    root->hard_links = 1;
    root->sectors = 0;
    root->flags = 0;
    root->direct_block[0] = (uint32_t)NULL;
    root->direct_block[1] = (uint32_t)NULL;
    root->direct_block[2] = (uint32_t)NULL;
    root->direct_block[3] = (uint32_t)NULL;
    root->direct_block[4] = (uint32_t)NULL;
    root->direct_block[5] = (uint32_t)NULL;
    root->direct_block[6] = (uint32_t)NULL;
    root->direct_block[7] = (uint32_t)NULL;
    root->direct_block[8] = (uint32_t)NULL;
    root->direct_block[9] = (uint32_t)NULL;
    root->direct_block[10] = (uint32_t)NULL;
    root->direct_block[11] = (uint32_t)NULL;
    root->singly_indirect_block = (uint32_t)NULL;
    root->doubly_indirect_block = (uint32_t)NULL;
    root->triply_indirect_block = (uint32_t)NULL;
    read_block(mem, (uint32_t)sb->superblock + 1);
    struct block_group_descriptor* bgd = (struct block_group_descriptor*)buffer;
    int groups = sb->blocks / sb->blocks_per_group;
    struct bitmap bm = {.size=sb->block_size, .map=(uint8_t*)bgd[0].inode_bitmap};
    set_one(&bm, 1);
}


void read_block(uint8_t* mem, uint32_t blockid) {
    struct superblock* sb = (struct superblock*)(mem+1024);
    int blocksize = sb->block_size;
    memcpy(buffer, mem+blockid*blocksize, blocksize);
}


void write_block(uint8_t* mem, uint32_t blockid) {
    struct superblock* sb = (struct superblock*)(mem+1024);
    int block_size = 2 << (sb->block_size + 10);
    memcpy(mem+blockid*block_size, buffer, block_size);
}


struct inode* find_inode_from_id(uint8_t* mem, size_t inode_id) {
    inode_id--;                                // inode id starts from 1
    struct superblock* sb = (struct superblock*)(mem+1024);
    int groups = sb->blocks / sb->blocks_per_group;
    int group = inode_id / sb->inodes_per_group;
    read_block(mem, sb->superblock + 1);       // Read block group descriptor table
    struct block_group_descriptor* bgd = (struct block_group_descriptor*)buffer;
    struct inode* inodes = (struct inode*)bgd[group].inode_table;
    return inodes + (inode_id % sb->inodes_per_group);
}


int read_path(char* filename) {
    int i;
    if (filename[0] == '/') filename++;
    for (i = 0; filename[i] != 0 && filename[i] != '/'; i++);
    return i; 
}


struct inode* follow_symbolic_link(uint8_t* mem, struct inode* link) {
    int size = link->low_size;
    char* target_path;
    if (size < 60) {
        target_path = (char*)(&link->direct_block[0]);
    } else {
        target_path = (char*)link->direct_block[0];
    }
    struct inode* target_inode = find_inode_from_path(mem, target_path);

    // If the target is a symbolic link itself, follow it again
    if (target_inode->type_permission >> 11 == INODE_TYPE_SYMBOLIC) {
        target_inode = follow_symbolic_link(mem, target_inode);
    }

    return target_inode;
}


struct iter_dir_st {
    struct directory_entry* entry;
    size_t remain_size;
    int current_block;
};


struct iter_dir_st* iter_directory(uint8_t* mem, struct inode* dir, struct iter_dir_st* state) {
    struct superblock* sb = (struct superblock*)(mem+1024);
    int dir_type = dir->type_permission >> 11;
    if (dir_type != INODE_TYPE_DIRECTORY) {
        if (state != NULL) {
            printf("Panic: Searching an inode that is not a directory!!\n");
            exit(-1);
        } else {
            return NULL;
        }
    }

    if (state == NULL) {
        if (dir->low_size == 0) return NULL;
        state = (struct iter_dir_st*)malloc(sizeof(struct iter_dir_st));
        state->entry = dir->direct_block[0];
        state->remain_size = dir->low_size;
        state->current_block = 0;
        return state;
    } else {
        state->remain_size -= state->entry->size;
        state->entry = (struct directory_entry*)((char*)state->entry + state->entry->size);
        if (state->remain_size <= 0) {
            free(state);
            return NULL;
        }
        if (state->entry->size == 0) {
            state->current_block++;
            // Move to the next block
            if (state->current_block <= 11) {
                uint32_t block = dir->direct_block[state->current_block];
                read_block(mem, block);
                state->entry = (struct directory_entry*)buffer;
            } else if (state->current_block <= SINGLY_UPNUM(sb)) {
                // uint8_t** block_table = dir->singly_indirect_block;

                // state->entry = (struct directory_entry*)*((char**)dir->singly_indirect_block + (state->current_block - 12));
            } else if (state->current_block <= DOUBLY_UPNUM(sb)) {
                // FIXME: 
                // state->entry = (struct directory_entry*)*((char**)dir->singly_indirect_block + (state->current_block - 12));
            }
        }
    }
}


struct inode* search_directory(uint8_t* mem, struct inode* dir, char* filename) {
    // FIXME: handle if directory entries exceeed one block
    int total_size = dir->low_size;
    struct directory_entry* entry = (struct directory_entry*)dir->direct_block[0];
    while (total_size > 0) {
        if (strcmp(filename, entry->name) == 0) {
            return find_inode_from_id(mem, entry->inode);
        }
        total_size -= entry->size;
        entry = (struct directory_entry*)((uint32_t)entry + entry->size);
    }
    return NULL;
}


struct inode* find_inode_from_path(uint8_t* mem, char* filename) {
    if (filename[0] != '/') {
        printf("Must enter an absolute path\n");
        exit(-1);
    }
    char *name = filename + 1;
    int length;
    char* part_name;
    struct inode* current = find_inode_from_id(mem, 2);          // Root inode
    while(filename[length = read_path(name)] != 0) {
        if (current->hard_links == 0) return NULL;               // File already deleted
        int type = current->type_permission >> 11;
        switch (type) {
        case INODE_TYPE_DIRECTORY:
            part_name = (char*)malloc((length+1) * sizeof(char));
            memcpy(part_name, name, length);
            part_name[length] = 0;
            current = search_directory(mem, current, part_name);
            if (current == NULL) {                               // Can't find the file in the directory
                return NULL;
            }
            free(part_name);
            break;
        case INODE_TYPE_SYMBOLIC:
            current = follow_symbolic_link(mem, current);
            break;
        default:
            printf("Unable to find inode of %s\n", filename);
            exit(-1);
        }
        name += length;
    }
    return current;
}


int log2_(int value) {
    int i;
    for (i = 0; value; i++) value >>= 1;
    return i - 1;
}

int to_decimal(char* string) {
    int n = 0;
    char ch;
    while ((ch = *(string++))) {
        n = n * 10 + ch - '0';
    }
    return n;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("mkfs filename size blocksize\n");
        exit(-1);
    }
    char* filename = argv[1];
    int size = to_decimal(argv[2]);
    int blocksize = to_decimal(argv[3]);
    mkfs(filename, size, blocksize);
}