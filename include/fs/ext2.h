#ifndef EXT2_H
#define EXT2_H
#include <stdint.h>

// File System States
#define FS_STATE_CLEAN       0x01           // File system is clean
#define FS_STATE_ERROR       0x02           // File system has errors

// Error Handling Methods
#define ERR_IGNORE           0x01           //	Ignore the error (continue on)
#define ERR_REMOUNT          0x02           //	Remount file system as read-only
#define ERR_PANIC            0x03           //	Kernel panic

// Creator Operating System IDs
#define CREATOR_LINUX        0x00
#define CREATOR_HURD         0x01
#define CREATOR_MASIX        0x02
#define CREATOR_FREEBSD      0x03
#define CREATOR_OTHER        0x04

// Optional Feature Flags
#define OPT_FEATURE_PREALLOC        0x01    // Preallocate some number of (contiguous?) blocks
                                            //(see byte 205 in the superblock) to a directory when
                                            // creating a new one (to reduce fragmentation?)
#define OPT_FEATURE_AFS             0x02    // AFS server inodes exist
#define OPT_FEATURE_JOURNAL         0x04    // File system has a journal (Ext3)
#define OPT_FEATURE_EXT_ATTRS       0x08    // Inodes have extended attributes
#define OPT_FEATURE_RESIZABLE       0x10    // File system can resize itself for larger partitions
#define OPT_FEATURE_HASH            0x20    // Directories use hash index

// Required Feature Flags
#define RQD_FEATURE_COMPRESSION     0x01    //	Compression is used
#define RQD_FEATURE_TYPE            0x02    //	Directory entries contain a type field
#define RQD_FEATURE_REPLAY          0x04    //  File system needs to replay its journal
#define RQD_FEATURE_JOURNAL         0x08    //  File system uses a journal device


//  Read-Only Feature Flags
#define RDO_FEATURE_SPARSE          0x01    //  Sparse superblocks and group descriptor tables
#define RDO_FEATURE_64BIT           0x02    //  File system uses a 64-bit file size
#define RDO_FEATURE_BINARYTREE      0x04    //  Directory contents are stored in the form of a Binary Tree


//  Inode Types
//  The type indicator occupies the top hex digit (bits 15 to 12) of this 16-bit field
#define INODE_TYPE_FIFO             0x1000  //  FIFO
#define INODE_TYPE_CHARACTER        0x2000  //	Character device
#define INODE_TYPE_DIRECTORY        0x4000  //	Directory
#define INODE_TYPE_BLOCK            0x6000  //  Block device
#define INODE_TYPE_REGULAR          0x8000  //	Regular file
#define INODE_TYPE_SYMBOLIC         0xA000  //	Symbolic link
#define INODE_TYPE_SOCKER           0xC000  //	Unix socket

//  Inode Permissions
//  Permissions occupy the bottom 12 bits of this 16-bit field
#define INODE_OTHER_EXECUTE         0x001
#define INODE_OTHER_WRITE           0x002
#define INODE_OTHER_READ            0x004
#define INODE_GROUP_EXECUTE         0x008
#define INODE_GROUP_WRITE           0x010
#define INODE_GROUP_READ            0x020
#define INODE_USER_EXECUTE          0x040
#define INODE_USER_WRITE            0x080
#define INODE_USER_READ             0x100
#define INODE_STICKY                0x200
#define INODE_SET_GID               0x400
#define INODE_SET_UID               0x800

// Inode Flags
#define INODE_SECURE_DELETION       0x00001         // Secure deletion (not used)
#define INODE_KEEP_COPY             0x00002         // Keep a copy of data when deleted (not used)
#define INODE_COMPRESSION           0x00004         // File compression (not used)
#define INODE_SYNCHRONOUS           0x00008         // Synchronous updates—new data is written immediately to disk
#define INODE_IMMUTABLE             0x00010         // Immutable file (content cannot be changed)
#define INODE_APPENDONLY            0x00020         // Append only
#define INODE_NODUMP                0x00040         // File is not included in 'dump' command
#define INODE_NOUPDATETIME          0x00080         // Last accessed time should not updated
#define INODE_HASH_INDEX_DIR        0x10000         // Hash indexed directory
#define INODE_AFS_DIR               0x20000         // AFS directory
#define INODE_JOURNAL_DATA          0x40000         // Journal file data


struct superblock {
    /* 1024 bytes */
    // Base fields
    uint32_t inodes;                // Total number of inodes in file system
    uint32_t blocks;                // Total number of blocks in file system
    uint32_t sublocks;              // Number of blocks reserved for superuser (see offset 80)
    uint32_t free_blocks;           // Total number of unallocated blocks
    uint32_t free_inodes;           // Total number of unallocated inodes
    uint32_t superblock;            // Block number of the block containing the superblock
    uint32_t block_size;            // log2 (block size) - 10. (In other words, the number to 
                                    // shift 1,024 to the left by to obtain the block size)
    uint32_t fragment_size;         // log2 (fragment size) - 10. (In other words, the number to
                                    // shift 1,024 to the left by to obtain the fragment size)
    uint32_t blocks_per_group;      // Number of blocks in each block group
    uint32_t fragments_per_group;   // Number of fragments in each block group
    uint32_t inodes_per_group;      // Number of inodes in each block group
    uint32_t mount_time;            // Last mount time (in POSIX time)
    uint32_t write_time;            // Last written time (in POSIX time)
    uint16_t num_mounts;            // Number of times the volume has been mounted since its last consistency check (fsck)
    uint16_t num_sc_fsck;           // Number of mounts allowed before a consistency check (fsck) must be done
    uint16_t magic;                 // Ext2 signature (0xef53), used to help confirm the presence of Ext2 on a volume
    uint16_t state;                 // File system state (see below)
    uint16_t errors;                // What to do when an error is detected (see below)
    uint16_t minor_version;         // Minor portion of version (combine with Major portion below to construct full version field)
    uint32_t fsck_time;             // POSIX time of last consistency check (fsck)
    uint32_t fsck_interval;         // Interval (in POSIX time) between forced consistency checks (fsck)
    uint32_t creator;               // Operating system ID from which the filesystem on this volume was created (see below)
    uint32_t major_version;         // Major portion of version (combine with Minor portion above to construct full version field)
    uint16_t uid;                   // User ID that can use reserved blocks
    uint16_t gid;                   // Group ID that can use reserved blocks
    // extended fields
    uint32_t first_inode;           // First non-reserved inode in file system. (In versions < 1.0, this is fixed as 11)
    uint16_t inode_size;            // Size of each inode structure in bytes. (In versions < 1.0, this is fixed as 128)
    uint16_t group;                 // Block group that this superblock is part of (if backup copy)
    uint32_t opt_features;          // Optional features present (features that are not required to read or write,
                                    // but usually result in a performance increase. see below)
    uint32_t rqd_features;          // Required features present (features that are required to be supported to read or write. see below)
    uint32_t rdo_features;          // Features that if not supported, the volume must be mounted read-only see below)
    char fsid[16];                  // File system ID (what is output by blkid)
    char volume_name[16];           // Volume name (C-style string: characters terminated by a 0 byte)
    char mnt_path[64];              // Path volume was last mounted to (C-style string: characters terminated by a 0 byte)
    uint32_t compr_alg;             // Compression algorithms used (see Required features above)
    uint8_t prealloc_blocks_files;  // Number of blocks to preallocate for files
    uint8_t prealloc_blocks_dirs;   // Number of blocks to preallocate for directories
    uint16_t unused;                // (Unused)
    char journal_id[16];            // Journal ID (same style as the File system ID above)
    uint32_t journal_inode;         // Journal inode
    uint32_t journal_device;        // Journal device
    uint32_t orphan_inodes;         // Head of orphan inode list
    uint8_t X[1024-236];            // (Unused)
} __attribute__((packed));


//  Block Group Descriptor
struct block_group_descriptor {
    /* 32 bytes */
    uint32_t block_bitmap;          //  Block address of block usage bitmap
    uint32_t inode_bitmap;          //  Block address of inode usage bitmap
    uint32_t inode_table;           //  Starting block address of inode table
    uint16_t num_free_blocks;       //  Number of unallocated blocks in group
    uint16_t num_free_inodes;       // 	Number of unallocated inodes in group
    uint16_t num_dirs;              // 	Number of directories in group
    uint8_t unused[14];             //  Unused
} __attribute__((packed));


//  Inode Data Structure
struct inode {
    /* 128 bytes */
    uint16_t type_permission;       //  Type and Permissions
    uint16_t uid;                   //  User ID
    uint32_t low_size;              // 	Lower 32 bits of size in bytes
    uint32_t atime;                 //  Last Access Time
    uint32_t ctime;                 //  Creation Time
    uint32_t mtime;                 //  Last Modification time
    uint32_t dtime;                 //  Deletion time
    uint16_t gid;                   //  Group ID
    uint16_t hard_links;            //  Count of hard links (directory entries) to this inode.
                                    //  When this reaches 0, the data blocks are marked as unallocated.
    uint32_t sectors;               //  Count of disk sectors (not Ext2 blocks) in use by this inode,
                                    //  not counting the actual inode structure nor directory entries linking to the inode.
    uint32_t flags;
    uint8_t ossv1[4];               // 	Operating System Specific value #1
    uint32_t direct_block[12];          // Direct Block Pointer 0 - 11
    uint32_t singly_indirect_block;     // Singly Indirect Block Pointer (Points to a block that is a list of block pointers to data)
    uint32_t doubly_indirect_block;     // Doubly Indirect Block Pointer (Points to a block that is a list of block pointers to Singly Indirect Blocks)
    uint32_t triply_indirect_block;     // Triply Indirect Block Pointer (Points to a block that is a list of block pointers to Doubly Indirect Blocks)
    uint32_t generation;                // Generation number (Primarily used for NFS)
    uint32_t extended_attrs;            // Extended attribute block (File ACL).
    uint32_t high_size;                 // Upper 32 bits of file size (if feature bit set) if it's a file, Directory ACL if it's a directory
    uint32_t fragment;                  // Block address of fragment
    uint8_t fragment_number;            // Fragment number
    uint8_t fragment_size;              // Fragment size
    uint16_t reserved1;                 // Reserved
    uint16_t high_uid;                  // Higher 16bit of 32bit user-id
    uint16_t high_gid;                  // Higher 16bit of 32bit group-id
    uint32_t reserved2;                 // Reserved
} __attribute__((packed));

struct directory_entry {
    uint32_t inode;
    uint16_t size;
    uint8_t low_name_length;
    uint8_t type_indicator;
    char name[0];
} __attribute__((packed));

#endif /* EXT2_H */