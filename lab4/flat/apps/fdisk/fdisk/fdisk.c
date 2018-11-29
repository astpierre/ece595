#include "usertraps.h"
#include "misc.h"

#include "fdisk.h"

dfs_superblock sb;
dfs_inode inodes[DFS_INODE_NMAX_NUM];
uint32 fbv[DFS_FBV_MAX_NUM_WORDS];

uint32 disk_bsize = 0;      // These are global in order to speed things up
uint32 disksize = 0;        // (i.e. fewer traps to OS to get the same number)

// Function to write a dfsblock to the disk
int FdiskWriteBlock(uint32 blocknum, char ** addr); 

void main (int argc, char *argv[])
{
    // Variable declaration
    int i,j;
    char diskblock_buffer[disk_blocksize()];
    char * ptr;
    
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
    Printf("   sizeof(dfs_inode)            = %d bytes\n",sizeof(dfs_inode)); 
    disksize = disk_size();     
    Printf("   disk_size()                  = %d bytes\n",disksize);
    for(i=0; i<disk_bsize;i++) diskblock_buffer[i]=0;
    disk_bsize = disk_blocksize();
    Printf("   disk_blocksize()             = %d bytes\n",disk_bsize);
    
    // 3. Invalidate filesystem before writing to it to make sure OS
    //    does not wipe out what we do here with old version in mem
    sb.valid = 0;
    Printf("  Initializing superblock...\n");
    sb.bsize = DFS_BLOCKSIZE;
    Printf("   sb.bsize                  = %d bytes\n",sb.bsize);
    sb.nblocks = disksize / sb.bsize;
    Printf("   sb.nblocks                = %d blocks\n",sb.nblocks);
    sb.ninodes = FDISK_NUM_INODES;
    Printf("   sb.ninodes                = %d inodes\n",sb.ninodes);
    sb.inodeBstart = FDISK_INODE_BLOCK_START;
    sb.fbvBstart = FDISK_FBV_BLOCK_START;
    sb.dataBstart = FDISK_FBV_BLOCK_START + (((sb.nblocks+31)/32*4) + (sb.bsize-1))/sb.bsize;
    Printf("  DLXOS File System (DFS) structure...\n");
    Printf("   Block 0                    = master boot record + sb\n");
    Printf("   Blocks %d --> %d            = arr inode structures\n",sb.inodeBstart,(sb.fbvBstart-1));
    Printf("   Blocks %d --> %d           = free block vector\n",sb.fbvBstart,(sb.dataBstart-1));
    Printf("   Blocks %d --> %d        = data blocks\n",sb.dataBstart,(sb.nblocks));
    
    // 4. Make sure the disk exists before doing anything else
    //    This creates a Linux file holding the DFS
    if(disk_create() == DISK_FAIL){  Printf("error in fdisk process\n");  }
    Printf("   disk_create()                = DISK_SUCCESS\n");

    Printf("  Writing all the inode blocks as not in use/empty...\n"); 
    // 5. Write all inodes as not in use & empty 
    for(i=0; i<sb.ninodes; i++)
    {
        inodes[i].inuse = 0;
        inodes[i].fsize = 0;
        inodes[i].fname[0] = '\0';
        for(j=0; j<DFS_INODE_BTABLE_SIZE; j++) inodes[i].btable[j] = -1;
        inodes[i].ibtable = -1;
        inodes[i].iibtable = -1;
    }
    ptr = (char *)inodes;
    for(i=sb.inodeBstart; i<sb.fbvBstart; i++) FdiskWriteBlock(i,&ptr);

    // 6. Next, setup free block vector (fbv) and write fbv to the disk
    Printf("  Clearing the free block vector...\n"); 
    fbv[0] = 0xFFFFFF00; //using 24 blocks for dfs
    for(i=1; i<DFS_FBV_MAX_NUM_WORDS; i++) fbv[i] = 0;  // Initialize by clearing all
    Printf("  Writing free block vector to disk...\n"); 
    ptr = (char *)fbv;
    for(i=sb.fbvBstart; i<sb.dataBstart; i++) FdiskWriteBlock(i,&ptr);


    // 7. Finally, setup superblock as valid filesystem & write to disk
    Printf("  Setting up superblock as valid and writing to disk block 1...\n"); 
    sb.valid = 1;
    // Since the boot record is totally zero'd out (by the disk_create()
    // system call, we want to write the superblock to physical block 1
    bcopy((char*)&sb, diskblock_buffer, sizeof(dfs_superblock));
    disk_write_block(FDISK_BOOT_FILESYSTEM_BLOCKNUM+1, diskblock_buffer);
    Printf("============================================================\n"); 
    Printf(" fdisk.c (PID: %d): Formattd DFS disk for %d bytes!\n", getpid(),disksize); 
    Printf("============================================================\n\n\n"); 
}

int FdiskWriteBlock(uint32 dfsblocknum, char ** addr)
{
    // Remember: dfsblocknum = DFS block num
    // # times to write to phys disk = sb.bsize / disk_bsize
    uint32 times_to_write = sb.bsize / disk_bsize;
    uint32 i, phydisk_blocknum;
    for(i=0; i<times_to_write; i++)
    {
        // Calculate the physical block number from dfsblocknum, i, times_to_write
        phydisk_blocknum = i+(dfsblocknum*times_to_write);
        // Write that tempblock to the disk block
        if(disk_write_block(phydisk_blocknum,*addr) == DISK_FAIL)
        {  
            Printf("Error writing physdisk blocknum %d!\n");
            return DISK_FAIL;
        }
        *addr = *addr + disk_bsize;
    }
    return DISK_SUCCESS;
}
