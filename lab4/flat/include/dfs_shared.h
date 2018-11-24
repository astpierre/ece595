#ifndef __DFS_SHARED__
#define __DFS_SHARED__

// --------------------------------------------------------
// Superblock type definition
typedef struct dfs_superblock {
    uint32 valid;
    uint32 dfs_blocksize;
    uint32 dfs_numblocks;
    uint32 dfs_inode_startblock;
    uint32 dfs_numinodes;
    uint32 dfs_freeblockvector_startblock; 
    uint32 dfs_data_startblock; 
} dfs_superblock;

// --------------------------------------------------------
// DFS block type definition
#define DFS_BLOCKSIZE 1024  // Integer mult of disk blocksize
typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;

// --------------------------------------------------------
// DFS Inode type definitions and constants
#define DFS_INODE_MAX_FILENAME_LENGTH 72
#define DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS 10
#define DFS_INODE_NUM_INDIRECT_ADDRESSED_BLOCKS 2
typedef struct dfs_inode {
    uint32 inuse;
    uint32 file_size; 
    char file_name[DFS_INODE_MAX_FILENAME_LENGTH];
    uint32 dir_addr_table[DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS];
    uint32 indir_addr_table[DFS_INODE_NUM_INDIRECT_ADDRESSED_BLOCKS];
    // Total size: 128 bytes
} dfs_inode;

#define DFS_MAX_FILESYSTEM_SIZE 0x10000000  // 64MB 
#define DFS_MAX_NUM_BLOCKS (DFS_MAX_FILESYSTEM_SIZE / DFS_BLOCKSIZE)
#define DFS_BITS_PER_BYTE 8
#define DFS_FBV_MAX_NUM_WORDS ((DFS_MAX_FILESYSTEM_SIZE/DFS_BLOCKSIZE)+31)/32
#define DFS_INODE_MAX_NUM 128
#define DFS_SUPERBLOCK_PHYSICAL_BLOCKNUM 1 // Where to write superblock on disk
#define DFS_FAIL -1
#define DFS_SUCCESS 1

#endif
