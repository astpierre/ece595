#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"

// Global file system parameters
static dfs_inode inodes[DFS_INODE_MAX_NUM];
static dfs_superblock sb;
static uint32 fbv[DFS_FBV_MAX_NUM_WORDS];

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }

// Using locks for the free block vector and the inodes
static lock_t lock_fbv;
static lock_t lock_inodes;

//-----------------------------------------------------------------
// DfsInavlidate marks the current version of the filesystem in
// memory as invalid.  This is really only useful when formatting
// the disk, to prevent the current memory version from overwriting
// what you already have on the disk when the OS exits.
//-----------------------------------------------------------------
void DfsInvalidate() 
{
    // Sets the valid bit of the superblock to 0
    sb.valid = 0;
}
void DfsValidate() 
{
    // Sets the valid bit of the superblock to 1
    sb.valid = 1;
}


//-----------------------------------------------------------------
// DfsFreeblockVectorChecker returns a 1 if block is inuse, and a
// 0 otherwise.
//-----------------------------------------------------------------
uint32 DfsFreeblockVectorChecker(uint32 blocknum) 
{
    uint32 wordsize = 32;
    if(fbv[blocknum/wordsize] & (1 << (blocknum % 32)))
    {
        // BLOCK NOT IN-USE
        return 0;
    }
    // BLOCK IN-USE
    else return 1;
}


//-----------------------------------------------------------------
// DfsFreeblockVectorSet sets a dfsblock to val in the freeblockV
//-----------------------------------------------------------------
void DfsFreeblockVectorSet(uint32 blocknum, uint32 val)
{
    // Set the value of blocknum in the FBV
    fbv[blocknum/32]=(fbv[blocknum/32]&invert(val<<(blocknum%32)))|(val<<(blocknum%32));
}


//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use
// locks where necessary.
//-----------------------------------------------------------------
uint32 DfsAllocateBlock() 
{
    // Initialize variables and parameters
    uint32 i;
    
    // Make sure that filesystem is already open
    if(sb.valid != 1) return DFS_FAIL;

    // Grab the lock
    while(LockHandleAcquire(lock_fbv) != SYNC_SUCCESS);
    
    // Use the freeblock vector checker to determine if allocated
    for(i=0; i<sb.dfs_numblocks; i+=32)
    {
        if(DfsFreeblockVectorChecker(i) != 0) 
        {
            // This block is free! lets allocate it!
            // Mark it in-use
            DfsFreeblockVectorSet(i,0);
            // Release the lock
            while(LockHandleRelease(lock_fbv) != SYNC_SUCCESS);
            return i;
        }
    }
    // Release the lock
    while(LockHandleRelease(lock_fbv) != SYNC_SUCCESS);
    return DFS_FAIL;
}


//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------
int DfsFreeBlock(uint32 blocknum) 
{
    // Make sure that filesystem is already open
    if(sb.valid != 1) return DFS_FAIL;

    // Grab the lock
    while(LockHandleAcquire(lock_fbv) != SYNC_SUCCESS);
    
    // Mark it !in-us
    DfsFreeblockVectorSet(blocknum,1);
    
    // Release the lock
    while(LockHandleRelease(lock_fbv) != SYNC_SUCCESS);

    return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------
int DfsReadBlock(uint32 blocknum, dfs_block *b) 
{
    // Initialize variables and parameters
    int bytes_read = 0;
    uint32 i;
    uint32 times_to_read;
    uint32 diskblocksize = (uint32)DiskBytesPerBlock();
    disk_block_imitation temp_diskblock;
    uint32 physdisk_blocknum;
    
    // Make sure that filesystem is already open
    if(sb.valid != 1) return DFS_FAIL;

    // Use the freeblock vector checker to determine if allocated
    if(DfsFreeblockVectorChecker(blocknum) != 1) return DFS_FAIL;

    // Read from disk in intervals of dfs_blocksize
    times_to_read = sb.dfs_blocksize / diskblocksize;
    for(i=0; i<times_to_read; i++)
    {
        physdisk_blocknum = i+(blocknum*times_to_read);
        bzero(temp_diskblock.data,diskblocksize);
        if(DiskReadBlock(physdisk_blocknum,(char*)&temp_diskblock) == DISK_FAIL)
        {  return DISK_FAIL;  }
        bcopy(temp_diskblock.data, &b->data[diskblocksize*i], diskblocksize);
        bytes_read+=diskblocksize;
    }

    // Return the number of bytes read
    return bytes_read;
}


//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.  
//-----------------------------------------------------------------
int DfsWriteBlock(uint32 blocknum, dfs_block *b)
{
    // Initialize variables and parameters
    int bytes_written = 0;
    uint32 i;
    uint32 times_to_write;
    uint32 diskblocksize = (uint32)DiskBytesPerBlock();
    disk_block_imitation temp_diskblock;
    uint32 physdisk_blocknum;
    
    // Make sure that filesystem is already open
    if(sb.valid != 1) return DFS_FAIL;

    // Use the freeblock vector checker to determine if allocated
    if(DfsFreeblockVectorChecker(blocknum) != 1) return DFS_FAIL;

    // Write to disk in intervals of dfs_blocksize
    times_to_write = sb.dfs_blocksize / diskblocksize;
    for(i=0; i<times_to_write; i++)
    {
        bcopy(&b->data[diskblocksize*i], temp_diskblock.data, diskblocksize);
        physdisk_blocknum = i+(blocknum*times_to_write);
        if(DiskWriteBlock(physdisk_blocknum,(char*)&temp_diskblock) == DISK_FAIL)
        {  return DISK_FAIL;  }
        bytes_written+=diskblocksize;
    }

    // Return the number of bytes written
    return bytes_written;
}


//-------------------------------------------------------------------
// DfsOpenFileSystem loads the file system metadata from the disk
// into memory.  Returns DFS_SUCCESS on success, and DFS_FAIL on 
// failure.
//-------------------------------------------------------------------
int DfsOpenFileSystem() 
{
    // Initialize variables and parameters
    int i;
    dfs_block temp_dfsblock;
    disk_block_imitation temp_diskblock;
    char * contig_inode_bytes;
    char * contig_fbv_bytes;

    
    // Check that filesystem is not already open
    if(sb.valid == 1) return DFS_FAIL;

    // Read superblock from disk 
    DiskReadBlock(1, &temp_diskblock);

    // Copy the data from the block we just read into the superblock in mem
    bcopy(temp_diskblock.data,(char*)&sb, sizeof(sb));

    // Read the inodes
    contig_inode_bytes = (char *) inodes; // congiguous copy
    for(i=sb.dfs_inode_startblock; i<sb.dfs_freeblockvector_startblock; i++)
    {
        DfsReadBlock(i,&temp_dfsblock);
        bcopy(temp_dfsblock.data,
              &(contig_inode_bytes[(i-sb.dfs_inode_startblock)*sb.dfs_blocksize]), 
              sb.dfs_blocksize);
    }

    // Read the free block vector
    contig_fbv_bytes = (char *) fbv; // congiguous copy
    for(i=sb.dfs_freeblockvector_startblock; i<sb.dfs_data_startblock; i++)
    {
        DfsReadBlock(i,&temp_dfsblock);
        bcopy(temp_dfsblock.data,
              &(contig_fbv_bytes[(i-sb.dfs_freeblockvector_startblock)*sb.dfs_blocksize]), 
              sb.dfs_blocksize);

    }

    // Change superblock to be invalid
    DfsInvalidate();

    // Write superblock back to disk
    bzero(temp_diskblock.data, DiskBytesPerBlock());
    bcopy((char *)&sb, temp_diskblock.data, sizeof(sb));
    DiskWriteBlock(1,&temp_diskblock);

    // Change it back to be valid in memory 
    DfsValidate();
    printf(" DfsOpenFileSystem(): DFS has successfully been opened\n");
    return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsModuleInit is called at boot time to initialize things and
// open the file system for use
//-----------------------------------------------------------------
void DfsModuleInit() 
{
    // Set file system as invalid
    DfsInvalidate();
    // Create the locks for synchronization
    lock_fbv = LockCreate();
    lock_inodes = LockCreate();
    
    // Open file system using DfsOpenFileSystem()
    DfsOpenFileSystem();

    // Later steps... initialize buffer cache-here.
}


//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's 
// version.
//-------------------------------------------------------------------
int DfsCloseFileSystem() 
{
    // Initialize variables and parameters
    int i;
    dfs_block temp_dfsblock;
    disk_block_imitation temp_diskblock;
    char * contig_inode_bytes;
    char * contig_fbv_bytes;

    
    // Check that filesystem is still open
    if(sb.valid != 1) return DFS_FAIL;

    // Read superblock from disk 
    DiskReadBlock(1, &temp_diskblock);

    // Copy the data from the block we just read into the superblock in mem
    bcopy(temp_diskblock.data,(char*)&sb, sizeof(sb));

    // Write the inodes to disk
    contig_inode_bytes = (char *) inodes; // congiguous copy
    for(i=sb.dfs_inode_startblock; i<sb.dfs_freeblockvector_startblock; i++)
    {
        bcopy(&(contig_inode_bytes[(i-sb.dfs_inode_startblock)*sb.dfs_blocksize]), 
              temp_dfsblock.data,
              sb.dfs_blocksize);
        DfsWriteBlock(i,&temp_dfsblock);
    }

    // Read the free block vector
    contig_fbv_bytes = (char *) fbv; // congiguous copy
    for(i=sb.dfs_freeblockvector_startblock; i<sb.dfs_data_startblock; i++)
    {
        bcopy(&(contig_fbv_bytes[(i-sb.dfs_freeblockvector_startblock)*sb.dfs_blocksize]), 
              temp_dfsblock.data,
              sb.dfs_blocksize);
        DfsWriteBlock(i,&temp_dfsblock);
    }

    // Write superblock back to disk
    bzero(temp_diskblock.data, DiskBytesPerBlock());
    bcopy((char *)&sb, temp_diskblock.data, sizeof(sb));
    DiskWriteBlock(1,&temp_diskblock);

    // Change superblock to be invalid
    DfsInvalidate();
    printf(" DfsCloseFileSystem(): DFS has successfully been closed\n");
    return DFS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Inode-based functions
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsInodeFilenameExists looks through all the inuse inodes for 
// the given filename. If the filename is found, return the handle 
// of the inode. If it is not found, return DFS_FAIL.
//-----------------------------------------------------------------
uint32 DfsInodeFilenameExists(char *filename) 
{
    // Initialize variables and parameters
    int i;

    // Check that filesystem is open
    if(sb.valid != 1) return DFS_FAIL;

    // Do a string compare on the in-use inodes filenames
    for(i=0; i<DFS_INODE_MAX_NUM; i++)
    {
        if(inodes[i].inuse == 1)
        {
            // This inode is in-use, lets compare filenames
            if(dstrncmp(filename,inodes[i].file_name, DFS_MAX_FILENAME_LENGTH) == 0)
            {  return i;  }
        }
    }
    return DFS_FAIL;
}


//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the 
// specified filename. If the filename exists, return the handle 
// of the inode. If it does not, allocate a new inode for this 
// filename and return its handle. Return DFS_FAIL on failure. 
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------
uint32 DfsInodeOpen(char *filename) 
{
    // Initialize variables and parameters
    uint32 i;
    uint32 inode_handle;

    // Check that filesystem is open
    if(sb.valid != 1) return DFS_FAIL;

    // Check if this filename exists
    if((inode_handle=DfsInodeFilenameExists(filename)) != DFS_FAIL) return inode_handle;

    // Grab the lock
    while(LockHandleAcquire(lock_inodes) != SYNC_SUCCESS);
    // We need to alloc new inode for filename
    for(i=0; i<DFS_INODE_MAX_NUM; i++)
    {
        if(inodes[i].inuse != 1)
        {
            // This inode is not in-use, lets give it to this new filename
            inodes[i].file_size = 0;
            inodes[i].inuse = 1;
            dstrncpy(inodes[i].file_name, filename,DFS_MAX_FILENAME_LENGTH);
            // Release the lock
            while(LockHandleRelease(lock_inodes) != SYNC_SUCCESS);
            return i;
        }
    }

    // Release the lock
    while(LockHandleRelease(lock_inodes) != SYNC_SUCCESS);
    return DFS_FAIL;
}


//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary, then mark 
// the inode as no longer in use. Use locks when modifying the 
// "inuse" flag in an inode.Return DFS_FAIL on failure, and 
// DFS_SUCCESS on success.
//-----------------------------------------------------------------
int DfsInodeDelete(uint32 handle) 
{
    // Initialize variables and parameters
    uint32 i;
    uint32 inode_handle;
    dfs_block temp_dfsblock;

    // Check that filesystem is open
    if(sb.valid != 1) return DFS_FAIL;

    // Check if this filename exists
    if(inodes[handle].inuse != 1) return DFS_FAIL; 

    // Grab the lock
    while(LockHandleAcquire(lock_inodes) != SYNC_SUCCESS);
    // We need to remove the inode data for this filename
    inodes[inode_handle].file_size = 0;
    inodes[inode_handle].inuse = 0;
    bzero(inodes[handle].file_name, DFS_MAX_FILENAME_LENGTH);

    // We need to free the indirect address blocks
    for(i=0; i<DFS_INODE_NUM_INDIRECT_ADDRESSED_BLOCKS; i++)
    {
        if(inodes[handle].indir_addr_table[i] != 0)
        {  DfsFreeBlock(inodes[handle].indir_addr_table[i]);  }
        inodes[handle].indir_addr_table[i] = 0;
    }

    // We need to free the direct addressed blocks (10)
    for(i=0; i<DFS_INODE_NUM_DIRECT_ADDRESSED_BLOCKS; i++)
    {
        if(inodes[handle].dir_addr_table[i] != 0)
        {  DfsFreeBlock(inodes[handle].dir_addr_table[i]);  }
        inodes[handle].dir_addr_table[i] = 0;
    }
    
    // Release the lock and return success
    while(LockHandleRelease(lock_inodes) != SYNC_SUCCESS);
    return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by 
// the inode handle, starting at virtual byte start_byte, copying 
// the data to the address pointed to by mem. Return DFS_FAIL on 
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------
int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) 
{

    // Check that filesystem is open
    if(sb.valid != 1) return DFS_FAIL;

    // Check if this filename exists
    if(inodes[handle].inuse != 1) return DFS_FAIL;


}


//-----------------------------------------------------------------
// DfsInodeWriteBytes writes num_bytes from the memory pointed to 
// by mem to the file represented by the inode handle, starting at 
// virtual byte start_byte. Note that if you are only writing part 
// of a given file system block, you'll need to read that block 
// from the disk first. Return DFS_FAIL on failure and the number 
// of bytes written on success.
//-----------------------------------------------------------------
int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes) 
{

}


//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file. 
// This is defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------
uint32 DfsInodeFilesize(uint32 handle) 
{
    // Check that filesystem is open
    if(sb.valid != 1) return DFS_FAIL;

    // Check if this filename exists
    if(inodes[handle].inuse != 1) return DFS_FAIL; 

    return inodes[handle].file_size;
}


//-----------------------------------------------------------------
// DfsInodeAllocateVirtualBlock allocates a new filesystem block 
// for the given inode, storing its blocknumber at index 
// virtual_blocknumber in the translation table. If the 
// virtual_blocknumber resides in the indirect address space, and 
// there is not an allocated indirect addressing table, allocate it. 
// Return DFS_FAIL on failure, and the newly allocated file system 
// block number on success.
//-----------------------------------------------------------------
uint32 DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) 
{
    // Initialize variables
    dfs_block temp_dfsblock;
    uint32i * indir_table;

    // Check that filesystem is open
    if(sb.valid != 1) return DFS_FAIL;

    // Check if this filename exists
    if(inodes[handle].inuse != 1) return DFS_FAIL; 

    // If the virtual_blocknum < 10, it is in direct addressed block
    if(virtual_blocknum < 10) 
    { 
        if(inodes[handle].dir_addr_table[virtual_blocknum] != 0)
        {  return inodes[handle].dir_addr_table[virtual_blocknum];  }
        inodes[handle].dir_addr_table[virtual_blocknum] = DfsAllocateBlock();
        return inodes[handle].dir_addr_table[virtual_blocknum];
    }

    // If not, then it must be an indirect address
    // We will need to read the indirect block from the disk
    bzero(temp_dfsblock.data,sb.dfs_blocksize);
    DfsReadBlock(inodes[handle].indir_addr_table, &temp_dfsblock);
    bcopy(temp_dfsblock.data,(char *)indir_table,sb.dfs_blocksize);
    virtual_blocknum -= 10;
    return indir_table[virtual_blocknum];

}


//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------
uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum) 
{
    // Initialize variables
    dfs_block temp_dfsblock;
    uint32 indir_table[sb.dfs_blocksize / 4];

    // Check that filesystem is open
    if(sb.valid != 1) return DFS_FAIL;

    // Check if this filename exists
    if(inodes[handle].inuse != 1) return DFS_FAIL; 

    // If the virtual_blocknum < 10, it is in direct addressed block
    if(virtual_blocknum < 10) return inodes[handle].dir_addr_table[virtual_blocknum];
    
    // If not, then it must be an indirect address
    // We will need to read the indirect block from the disk
    bzero(temp_dfsblock.data,sb.dfs_blocksize);
    DfsReadBlock(inodes[handle].indir_addr_table, &temp_dfsblock);
    bcopy(temp_dfsblock.data,(char *)indir_table,sb.dfs_blocksize);
    virtual_blocknum -= 10;
    return indir_table[virtual_blocknum];
}
