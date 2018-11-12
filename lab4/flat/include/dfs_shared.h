#ifndef __DFS_SHARED__
#define __DFS_SHARED__

typedef struct dfs_superblock {
  // STUDENT: put superblock internals here
} dfs_superblock;

#define DFS_BLOCKSIZE 1024  // Must be an integer multiple of the disk blocksize

typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;


/*
#define DFS_INODE_MAX_FILENAME_LENGTH 
#define DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS
#define DFS_INODE_NUM_INDIRECT_ADDRESSED_BLOCKS
#define DFS_INODE_MAX_VIRTUAL_BLOCKNUM 
*/
typedef struct dfs_inode {
  // STUDENT: put inode structure internals here
  // IMPORTANT: sizeof(dfs_inode) MUST return 128 in order to fit in enough
  // inodes in the filesystem (and to make your life easier).  To do this, 
  // adjust the maximumm length of the filename until the size of the overall inode 
  // is 128 bytes.
} dfs_inode;

#define DFS_MAX_FILESYSTEM_SIZE 0x10000000  // 64MB 
#define DFS_MAX_NUM_BLOCKS (DFS_MAX_FILESYSTEM_SIZE / DFS_BLOCKSIZE)

/*
#define DFS_BITS_PER_BYTE
#define DFS_FBV_BITS_PER_ENTRY
#define DFS_FBV_MAX_NUM_WORDS 

#define DFS_INODE_MAX_NUM_FILESYSTEM_BLOCKS 
#define DFS_INODE_MAX_NUM 
*/

#define DFS_SUPERBLOCK_PHYSICAL_BLOCKNUM 1 // Where to write superblock on the disk

#define DFS_FAIL -1
#define DFS_SUCCESS 1



#endif
