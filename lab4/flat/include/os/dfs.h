#ifndef __DFS_H__
#define __DFS_H__

#include "dfs_shared.h"

// Function prototypes

void DfsInvalidate();
void DfsValidate();
uint32 DfsFreeblockVectorChecker(uint32 blocknum);
void DfsFreeblockVectorSet(uint32 blocknum, uint32 val);
uint32 DfsAllocateBlock();
int DfsFreeBlock(uint32 blocknum);
int DfsReadBlock(uint32 blocknum, dfs_block *b); 
int DfsWriteBlock(uint32 blocknum, dfs_block *b);
int DfsOpenFileSystem();
void DfsModuleInit();
int DfsCloseFileSystem();
uint32 DfsInodeFilenameExists(char *filename);
uint32 DfsInodeOpen(char *filename);
int DfsInodeDelete(uint32 handle);
int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes);
int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes);
uint32 DfsInodeFilesize(uint32 handle);
uint32 DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum);
uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum);


// Disk block imitation
typedef struct disk_block_imitation {
    char data[512];
} disk_block_imitation;

#endif
