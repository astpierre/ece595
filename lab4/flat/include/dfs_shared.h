#ifndef __DFS_SHARED__
#define __DFS_SHARED__

// --------------------------------------------------------
// Superblock type definition
typedef struct dfs_superblock {
    int valid;
    int bsize;
    int nblocks;
    int ninodes;
    int inodeBstart;
    int fbvBstart; 
    int dataBstart; 
} dfs_superblock;

// --------------------------------------------------------
// DFS block type definition
// DFS disk blocksize = 512 bytes (int multiple == 2)
#define DFS_BLOCKSIZE 1024  // Integer mult of disk blksze
typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;

// --------------------------------------------------------
// DFS Inode type definitions and constants
#define DFS_INODE_MAX_FNAME_LENGTH 72
#define DFS_INODE_BTABLE_SIZE 10
typedef struct dfs_inode {
    int inuse;
    int fsize; 
    char fname[DFS_INODE_MAX_FNAME_LENGTH];
    int btable[DFS_INODE_BTABLE_SIZE];
    int ibtable;
    int iibtable;
    // Total size: 128 bytes
    // 16+40 = 56
    // 128-56 = 72
} dfs_inode;

#define DFS_MAX_FILESYSTEM_SIZE 0x10000000  // 64MB 
#define DFS_MAX_NUM_BLOCKS (DFS_MAX_FILESYSTEM_SIZE / DFS_BLOCKSIZE)
// 8 dfs blocks for fbv => 1024*8=8192bytes / 4(bytes/word) = 1024
#define DFS_FBV_MAX_NUM_WORDS (DFS_BLOCKSIZE*8)/4
#define DFS_INODE_NMAX_NUM 128
#define DFS_SB_PBLOCK 1 // where write sb on disk
#define DFS_FAIL -1
#define DFS_SUCCESS 1

#endif
