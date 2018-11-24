#include "usertraps.h"
#include "misc.h"

#include "fdisk.h"

dfs_superblock sb;
dfs_inode inodes[DFS_INODE_MAX_NUM];
uint32 freeblockvector[DFS_FBV_MAX_NUM_WORDS];

uint32 physdisk_block_size = 0; // These are global in order to speed things up
uint32 physdisk_size = 0;       // (i.e. fewer traps to OS to get the same number)

// Function to write a dfsblock to the disk
int FdiskWriteBlock(uint32 blocknum, dfs_block *b); 

void main (int argc, char *argv[])
{
    // Variable declaration
    dfs_block temp_dfsblock;
    int i;
    int idx;
    
    Printf("\n\n"); 
    Printf("============================================================\n"); 
    Printf(" fdisk.c (PID: %d): Q1, user program to format disk\n", getpid()); 
    Printf("============================================================\n"); 

    // 1. argc check
    if (argc != 1) 
    {  Printf("ERROR: no input expected, %s\n", argv[0]); Exit();  }
    
    // 2. Use sys calls to calculate basic filesystem parameters (GLOBALS)
    Printf("  Calculating essential DFS parameters using system calls...\n");
    if(sizeof(dfs_inode) != 128)
    {  Printf("ERROR: Size of inode isn't 128 bytes %d\n"); Exit();  }
    Printf("   sizeof(dfs_inode)     = %d bytes\n",sizeof(dfs_inode)); 
    physdisk_size = disk_size();     
    Printf("   disk_size()           = %d bytes\n",physdisk_size);
    physdisk_block_size = disk_blocksize();
    Printf("   disk_blocksize()      = %d bytes\n",physdisk_block_size);

    // 3. Make sure the disk exists before doing anything else
    //    This creates a Linux file holding the DFS
    if(disk_create() == DISK_FAIL){  Printf("error in fdisk process\n");  }
    Printf("   disk_create()         = DISK_SUCCESS\n");
    
    // 4. Invalidate filesystem before writing to it to make sure OS
    //    does not wipe out what we do here with old version in mem
    sb.valid = 0;
    Printf("  Initializing superblock...\n");
    sb.dfs_blocksize = FDISK_DFS_BLOCKSIZE;
    Printf("   sb.dfs_blocksize      = %d bytes\n",sb.dfs_blocksize);
    sb.dfs_numblocks = physdisk_size / sb.dfs_blocksize;
    Printf("   sb.dfs_numblocks      = %d blocks\n",sb.dfs_numblocks);
    sb.dfs_numinodes = FDISK_NUM_INODES;
    Printf("   sb.dfs_numinodes      = %d inodes\n",sb.dfs_numinodes);
    sb.dfs_inode_startblock = FDISK_INODE_BLOCK_START;
    sb.dfs_freeblockvector_startblock = FDISK_FBV_BLOCK_START;
    sb.dfs_data_startblock = FDISK_FBV_BLOCK_START + 
                             (((sb.dfs_numblocks+31)/32*4) + 
                             (sb.dfs_blocksize-1))/sb.dfs_blocksize;
    Printf("  DLXOS File System (DFS) structure...\n");
    Printf("   Block 0               = master boot record and superblock\n");
    Printf("   Blocks %d --> %d      = array of inode structures\n",sb.dfs_inode_startblock,
                                                                (sb.dfs_freeblockvector_startblock-1));
    Printf("   Blocks %d --> %d      = free block vector\n",sb.dfs_freeblockvector_startblock,
                                                                (sb.dfs_data_startblock-1));
    Printf("   Blocks %d --> %d      = data blocks\n",sb.dfs_data_startblock,(sb.dfs_numblocks));

    Printf("  Writing all the inode blocks as not in use/empty...\n"); 
    // 5. Write all inodes as not in use & empty 
    for(i=sb.dfs_inode_startblock; i<sb.dfs_freeblockvector_startblock; i++)
    {
        // These are dfsblocks 1 through 16
        bzero((char*)&temp_dfsblock.data,sb.dfs_blocksize);
        FdiskWriteBlock(i,&temp_dfsblock);
    }

    // 6. Next, setup free block vector (fbv) and write fbv to the disk
    Printf("  Clearing the free block vector...\n"); 
    for(i=0; i<DFS_FBV_MAX_NUM_WORDS; i++)
    {
        // Initialize by clearing all
        freeblockvector[i] = 0;
    }

    Printf("  Asserting free block vector bits for the data blocks...\n"); 
    for(i=sb.dfs_data_startblock; i<sb.dfs_numblocks; i++)
    {
        // Set freeblockvector bits for data blocks to 'inuse'
        freeblockvector[i/32]=(freeblockvector[i/32]&((1 << (i%32))^0xFFFFFFFF))|
                              (1 << i%32);
    }

    Printf("  Writing free block vector to disk...\n"); 
    for(i=sb.dfs_freeblockvector_startblock; i<sb.dfs_data_startblock; i++)
    {
        // Copy over the free block to that temp DFS block and write
        idx = (i - sb.dfs_freeblockvector_startblock) * sb.dfs_blocksize;
        bcopy((char*)&freeblockvector[idx],temp_dfsblock.data,sb.dfs_blocksize);
        FdiskWriteBlock(i,&temp_dfsblock);
    }

    // 7. Finally, setup superblock as valid filesystem & write to disk
    Printf("  Setting up superblock as valid and writing to disk block 1...\n"); 
    sb.valid = 1;
    bzero(temp_dfsblock.data,sb.dfs_blocksize);
    // Since the boot record is totally zero'd out (by the disk_create()
    // system call, we want to write the superblock to physical block 1
    bcopy((char*)&sb, &(temp_dfsblock.data[physdisk_block_size]),sizeof(dfs_superblock));
    FdiskWriteBlock(FDISK_BOOT_FILESYSTEM_BLOCKNUM, &temp_dfsblock);
    Printf("============================================================\n"); 
    Printf(" fdisk.c (PID: %d): Successfully formattd DFS disk for %d bytes\n", getpid(),physdisk_size); 
    Printf("============================================================\n\n\n"); 
}

int FdiskWriteBlock(uint32 blocknum, dfs_block * b)
{
    // Remember: blocknum = DFS block num
    // # times to write to phys disk = sb.dfs_blocksize / physdisk_block_size
    uint32 times_to_write = sb.dfs_blocksize / physdisk_block_size;
    uint32 i;
    uint32 physdisk_blocknum;
    disk_block_imitation temp_diskblock;
    for(i=0; i<times_to_write; i++)
    {
        // Copy the first physdisk_block_size bytes into a tempblock from b
        bcopy(&(b->data[physdisk_block_size * i]), temp_diskblock.data, physdisk_block_size);
        // Calculate the physical block number from blocknum, i, times_to_write
        physdisk_blocknum = i+(blocknum*times_to_write);
        // Write that tempblock to the disk block
        if(disk_write_block(physdisk_blocknum,&temp_diskblock) == DISK_FAIL)
        {  
            Printf("Error writing physdisk blocknum %d!\n");
            return DISK_FAIL;
        }
    }
    return DISK_SUCCESS;
}
