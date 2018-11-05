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


uint8_t* buffer;


void init_superblock(uint8_t*, int, int);
void init_block_descriptors(uint8_t*);
void init_block_groups(uint8_t*);
void create_root_inode(uint8_t*);
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
    int timestamp = time(&tm);
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
    sb.mount_time = timestamp;
    sb.write_time = timestamp;
    sb.num_mounts = 0;
    sb.num_sc_fsck = 0;
    sb.magic = 0xEF53;
    sb.state = FS_STATE_CLEAN;
    sb.errors = ERR_PANIC;
    sb.minor_version = 0;
    sb.fsck_interval = 10;
    sb.fsck_time = timestamp;
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