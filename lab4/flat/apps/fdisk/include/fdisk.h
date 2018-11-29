#ifndef __FDISK_H__
#define __FDISK_H__

typedef unsigned int uint32;

#include "dfs_shared.h" // This gets us structures from FS drv

#define FDISK_INODE_BLOCK_START 1 // Start after supblock

// Number of file system blocks to use for inodes
#define FDISK_NUM_INODES 128
#define FDISK_INODE_NUM_BLOCKS FDISK_NUM_INODES*sizeof(dfs_inode)/DFS_BLOCKSIZE // = 16blocks
#define FDISK_FBV_BLOCK_START FDISK_INODE_NUM_BLOCKS+FDISK_INODE_BLOCK_START // = 17
// Where boot record and superblock reside in the filesystem
#define FDISK_BOOT_FILESYSTEM_BLOCKNUM 0
#ifndef NULL
#define NULL (void *)0x0
#endif

//STUDENT: define additional parameters here, if any
#define FDISK_DFS_BLOCKSIZE DFS_BLOCKSIZE

#endif
