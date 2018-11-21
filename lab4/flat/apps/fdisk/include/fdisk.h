#ifndef __FDISK_H__
#define __FDISK_H__

typedef unsigned int uint32;

#include "dfs_shared.h" // This gets us structures and #define's from main filesystem driver

#define FDISK_INODE_BLOCK_START 1 // Starts after super block (which is in file system block 0, physical block 1)

// Number of file system blocks to use for inodes
#define FDISK_INODE_NUM_BLOCKS (sizeof(dfs_inode))
#define FDISK_NUM_INODES  
#define FDISK_FBV_BLOCK_START 
// Where boot record and superblock reside in the filesystem
#define FDISK_BOOT_FILESYSTEM_BLOCKNUM 0 
#ifndef NULL
#define NULL (void *)0x0
#endif

//STUDENT: define additional parameters here, if any

#endif
