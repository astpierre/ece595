#ifndef __DISK_H__
#define __DISK_H__

// Name of file which represents the "hard disk".
// Chose 77 because not sure what my group num is
#define DISK_FILENAME "/tmp/ee469g77.img"

// Number of bytes in one physical disk block
#define DISK_BLOCKSIZE 512 

// Type definitions for disk blocks
typedef struct disk_block {
    char data[DISK_BLOCKSIZE]; // DISK_BLOCKSIZE % 4 = 0 (byte alignment)
} disk_block;

// Total size of this disk, in units of 512-byte blocks
//  64-megabytes / 512-bytes = 125,000 blocks
#define DISK_NUMBLOCKS 125000

#define DISK_SUCCESS 1
#define DISK_FAIL -1

int DiskBytesPerBlock();
int DiskSize();
int DiskCreate();
int DiskWriteBlock (uint32 blocknum, disk_block *b);
int DiskReadBlock (uint32 blocknum, disk_block *b);

#endif
