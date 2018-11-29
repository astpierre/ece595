#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"

// Global file system parameters
static dfs_inode inodes[DFS_INODE_NMAX_NUM];
static dfs_superblock sb;
static int fbv[DFS_FBV_MAX_NUM_WORDS];
static int dfsOpen = 0;
static int negativeone = 0xFFFFFFFF;
static inline int invert(int n) { return n ^ negativeone; }
inline uint32 DFS_PHY_RATIO(){ return sb.bsize / DiskBytesPerBlock(); }
inline uint32 DFS_TO_PHY_BNUM(uint32 n){ return (n*DFS_PHY_RATIO()); }

// Using locks for the free block vector and the inodes
static lock_t lock_fbv;
static lock_t lock_inodes;


// DfsInvalidate ==========================================
// Marks the current version of the file system in memory
// as invalid. This is really only useful when formatting
// the disk, to prevent the current memory version from 
// overwriting what you already have on the disk when OS
// exits.
// ========================================================
void DfsInvalidate() 
{
    sb.valid = 0; // Sets the valid bit of the superblock to 0
}

// DfsModuleInit ==========================================
// Is called at boot time t initialize things and open the
// file system for use.
// ========================================================
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

// DfsFBVChecker ==========================================
// Looks in the free block vector and returns the status
// of the dfs block corresponding to block number. Returns 
// 0 if inuse, 1 if free.
// ========================================================
uint32 DfsFBVChecker(uint32 blocknum) 
{
    int fbvPacket = blocknum >> 5;      // blocknum / 32 
    int fbvPosition = blocknum & 0x1F;  // blocknum bitwise AND

    // Check if blocknum is allocated, the bitwise AND true
    return (fbv[fbvPacket] & (0x80000000 >> fbvPosition));
}

// DfsFBVSet ==============================================
// Sets a dfs block entry in the free block vector to the
// value specified as input to the helper (val).
// ========================================================
void DfsFBVSet(uint32 blocknum, uint32 val)
{
    int fbvPacket = blocknum >> 5;      // blocknum / 32 
    int fbvPosition = blocknum & 0x1F;  // blocknum bitwise AND

    // Set the value of blocknum in the FBV
    if(val == 0) // CLEAR (FREEING) (mark free)
    fbv[fbvPacket] &= invert(1<<(31 - fbvPosition));
    else if(val == 1) // SET (ALLOCATING) (mark inuse)
    fbv[fbvPacket] |= (1<<(31 - fbvPosition));
}

// DfsAllocateBlock =======================================
// Allocates a DFS block for use. Returns DFS_FAIL on 
// failure, and the allocated block number on proper alloc
// ========================================================
uint32 DfsAllocateBlock() 
{
    // Initialize variables and parameters
    int i=0,j=0,pack=0,pos=0;
    
    // Make sure that filesystem is already open
    if(sb.valid != 1 || dfsOpen != 1) return DFS_FAIL;

    // Grab the lock
    while(LockHandleAcquire(lock_fbv) != SYNC_SUCCESS);
    
    // First let's find a packet with at least one zero
    while(fbv[i] == 0xFFFFFFFF)
    {  i++; if(i >= DFS_FBV_MAX_NUM_WORDS) return DFS_FAIL; pack=i;  }
    // Now, let's find that zero bit (first apperance)
    for(j=0; j<32; j++)
    {
        if((fbv[pack]<<j)>>(31-j)==0){  pos = j; break;  }
    }
    fbv[pack] = fbv[pack] | (1 << (31-pos));
    while(LockHandleRelease(lock_fbv) != SYNC_SUCCESS);
    return 32*pack+pos;
}

// DfsFreeBlock ===========================================
// Deallocates a DFS block. Returns DFS_FAIL on failure,
// and DFS_SUCCESS on good freeing.
// ========================================================
int DfsFreeBlock(uint32 blocknum) 
{
    // Make sure that filesystem is already open
    if(sb.valid != 1 || dfsOpen != 1) return DFS_FAIL;

    // Grab the lock
    while(LockHandleAcquire(lock_fbv) != SYNC_SUCCESS);
    
    // Mark it !inuse
    DfsFBVSet(blocknum,0);
    
    // Release the lock
    while(LockHandleRelease(lock_fbv) != SYNC_SUCCESS);
    return DFS_SUCCESS;
}

// DfsReadBlock ==========================================
// Reads an allocated DFS block from the disk (which could 
// span multiple physical disk blocks). The block must be 
// allocated in order to read from it. Returns DFS_FAIL on 
// failure, and the number of bytes read on success.
// ========================================================
int DfsReadBlock(uint32 blocknum, dfs_block *b) 
{
    // Initialize variables and parameters
    int bytes_read=0, i=0;
    uint32 phydisk_blocknum = DFS_TO_PHY_BNUM(blocknum);
    disk_block diskblock_buffer;
    char * ptr = b->data;
    
    // Make sure that filesystem is already open
    if(sb.valid != 1) 
    {  printf("ERR: sb.valid != 1\n"); return DFS_FAIL;  }

    // Use the freeblock vector checker to determine if allocated
    //if(DfsFBVChecker(blocknum) == 0)
    //{  printf("ERR: fbv said block isn't allocated\n"); return DFS_FAIL;  }

    // Read from disk in intervals of physical disk's blocksize
    for(i=phydisk_blocknum; i<(phydisk_blocknum + DFS_PHY_RATIO()); i++)
    {
        if(DiskReadBlock(i, &diskblock_buffer) != DISK_BLOCKSIZE)
        {  printf("ERR: DiskReadBlock didnt read number of disk block bytes\n"); return DFS_FAIL;  }
        bcopy(diskblock_buffer.data, ptr, DISK_BLOCKSIZE);
        bytes_read+=DISK_BLOCKSIZE;
        ptr+=DISK_BLOCKSIZE;   
    }
    return bytes_read;
}

// DfsWriteBlock ==========================================
// Writes an allocated DFS block to the disk (which could 
// span multiple physical disk blocks). The block must be 
// allocated in order to write to it. Returns DFS_FAIL on 
// failure, and the number of bytes written on success.
// ========================================================
int DfsWriteBlock(uint32 blocknum, dfs_block *b)
{
    // Initialize variables and parameters
    int bytes_written=0, i=0;
    uint32 phydisk_blocknum = DFS_TO_PHY_BNUM(blocknum);
    disk_block diskblock_buffer;
    char * ptr = b->data;
    
    // Make sure that filesystem is already open
    if(sb.valid != 1) 
    {  printf("ERR: sb.valid != 1\n"); return DFS_FAIL;  }

    // Use the freeblock vector checker to determine if allocated
    //printf("DFS_PHY_RATIO: %d\n",DFS_PHY_RATIO());
    //if(DfsFBVChecker(blocknum) != 0)
    //{  printf("ERR: fbv said block isn't allocated\n"); return DFS_FAIL;  }

    // Read from disk in intervals of physical disk's blocksize
    for(i=phydisk_blocknum; i<(phydisk_blocknum + DFS_PHY_RATIO()); i++)
    {
        bcopy(ptr, diskblock_buffer.data, DISK_BLOCKSIZE);
        if(DiskWriteBlock(i, &diskblock_buffer) != DISK_BLOCKSIZE)
        {  printf("ERR: DiskWriteBlock didnt write number of disk block bytes\n"); return DFS_FAIL;  }
        bytes_written+=DISK_BLOCKSIZE;
        ptr+=DISK_BLOCKSIZE;   
    }
    return bytes_written;
}

// DfsOpenFileSystem ======================================
// Loads the fil system metadata from the disk into 
// memory. Returns DFS_FAIL on failure. 
// ========================================================
int DfsOpenFileSystem() 
{
    // Initialize variables and parameters
    int i=0;
    disk_block diskblock_buffer;
    char * ptr;
    
    // Check that filesystem is not already open
    if(dfsOpen == 1) return DFS_FAIL;
    dfsOpen = 1; // opening now

    // Read superblock from disk
    if(DiskReadBlock(1, &diskblock_buffer) != DISK_BLOCKSIZE) 
    {  printf("ERR: DiskReadBlock didnt read number of disk block bytes\n"); return DFS_FAIL;  }

    // Copy the data from the block we just read into the superblock in mem
    bcopy(diskblock_buffer.data, (char *)(&sb), sizeof(dfs_superblock));

    // Read the inodes 
    ptr = (char *)inodes; // destination address
    for(i=DFS_TO_PHY_BNUM(sb.inodeBstart); i<DFS_TO_PHY_BNUM(sb.fbvBstart); i++)
    {
        if(DiskReadBlock(i, &diskblock_buffer) != DISK_BLOCKSIZE) 
        {  printf("ERR: DiskReadBlock didnt read number of disk block bytes\n"); return DFS_FAIL;  }
        bcopy(diskblock_buffer.data, ptr, DISK_BLOCKSIZE);
        ptr+=DISK_BLOCKSIZE;   
    }

    // Read the free block vector
    ptr = (char *)fbv; // destination address
    for(i=DFS_TO_PHY_BNUM(sb.fbvBstart); i<DFS_TO_PHY_BNUM(sb.dataBstart); i++)
    {
        if(DiskReadBlock(i, &diskblock_buffer) != DISK_BLOCKSIZE) 
        {  printf("ERR: DiskReadBlock didnt read number of disk block bytes\n"); return DFS_FAIL;  }
        bcopy(diskblock_buffer.data, ptr, DISK_BLOCKSIZE);
        ptr+=DISK_BLOCKSIZE;   
    }

    // Change superblock to be invalid
    DfsInvalidate();

    // Write superblock back to disk
    bzero(diskblock_buffer.data, DISK_BLOCKSIZE);
    bcopy((char *)(&sb), diskblock_buffer.data, sizeof(dfs_superblock));
    if(DiskWriteBlock(1, &diskblock_buffer) != DISK_BLOCKSIZE)
    {  printf("ERR: DiskWriteBlock didnt write number of disk block bytes\n"); return DFS_FAIL;  }

    // Change it back to be valid in memory 
    sb.valid = 1;
    printf(" DfsOpenFileSystem(): DFS has successfully been opened\n");
    return DFS_SUCCESS;
}

// DfsCloseFileSystem ======================================
// Writes the current memory version of the file system
// metadata to the disk, and invalidates mem's version.
// ========================================================
int DfsCloseFileSystem() 
{
    // Initialize variables and parameters
    int i=0;
    disk_block diskblock_buffer;
    char * ptr;
    
    // Check that filesystem is not already closed
    if(dfsOpen == 0) return DFS_SUCCESS;
    dfsOpen = 0; // closing now

    // Write back the inodes 
    ptr = (char *)inodes; // reference address
    for(i=DFS_TO_PHY_BNUM(sb.inodeBstart); i<DFS_TO_PHY_BNUM(sb.fbvBstart); i++)
    {
        bcopy(ptr, diskblock_buffer.data, DISK_BLOCKSIZE);
        if(DiskWriteBlock(i, &diskblock_buffer) != DISK_BLOCKSIZE) 
        {  printf("ERR: DiskWriteBlock didnt write number of disk block bytes\n"); return DFS_FAIL;  }
        ptr+=DISK_BLOCKSIZE;   
    }

    // Write back the free block vector
    ptr = (char *)fbv; // reference address
    for(i=DFS_TO_PHY_BNUM(sb.fbvBstart); i<DFS_TO_PHY_BNUM(sb.dataBstart); i++)
    {
        bcopy(ptr, diskblock_buffer.data, DISK_BLOCKSIZE);
        if(DiskWriteBlock(i, &diskblock_buffer) != DISK_BLOCKSIZE) 
        {  printf("ERR: DiskWriteBlock didnt write number of disk block bytes\n"); return DFS_FAIL;  }
        ptr+=DISK_BLOCKSIZE;   
    }

    // Change superblock to be valid
    sb.valid = 1;

    // Write superblock back to disk
    bzero(diskblock_buffer.data, DISK_BLOCKSIZE);
    bcopy((char *)(&sb), diskblock_buffer.data, sizeof(dfs_superblock));
    if(DiskWriteBlock(1, &diskblock_buffer) != DISK_BLOCKSIZE)
    {  printf("ERR: DiskWriteBlock didnt write number of disk block bytes\n"); return DFS_FAIL;  }
    printf(" DfsCloseFileSystem(): DFS has successfully been closed\n");
    return DFS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Inode-based functions
///////////////////////////////////////////////////////////////////////////////

// DfsInodeFilenameExists =================================
// Looks through all the inuse inodes for the given 
// filename. If the filename is found, return the handle
// of the inode. Else, return DFS_FAIL.
// ========================================================
uint32 DfsInodeFilenameExists(char *filename) 
{
    // Initialize variables and parameters
    int i;

    // Check that filesystem is open
    if(sb.valid != 1 || dfsOpen != 1) return DFS_FAIL;

    // Do a string compare on the in-use inodes filenames
    for(i=0; i<DFS_INODE_NMAX_NUM; i++)
    {
        if(inodes[i].inuse == 1)
        {
            // This inode is in-use, lets compare filenames
            if(dstrncmp(filename, inodes[i].fname, dstrlen(filename)) == 0)
            {  return i;  }
        }
    }
    return DFS_FAIL;
}

// DfsInodeOpen ===========================================
// Search the list of all inuse inodes for the specified 
// filename. If exists, return the handle of the inode. 
// Else, allocate a new inode for this filename and return
// its handle. Return DFS_FAIL on failure. Remember to
// use locks whenever you allocate a new inode. 
// ========================================================
uint32 DfsInodeOpen(char * filename) 
{
    // Initialize variables and parameters
    int i=0;
    uint32 inode_handle;

    // Check that filesystem is open
    if(sb.valid != 1 || dfsOpen != 1) return DFS_FAIL;

    // Check if this filename exists
    if((inode_handle=DfsInodeFilenameExists(filename)) != DFS_FAIL) return inode_handle;

    // If here, we need to allocate an inode for the filename
    for(i=0; i<DFS_INODE_NMAX_NUM; i++) {if(inodes[i].inuse != 1) {  inode_handle = i; break;  }}
    
    // Let's grab the lock
    while(LockHandleAcquire(lock_inodes) != SYNC_SUCCESS);
    inodes[inode_handle].fsize = 0;
    inodes[inode_handle].inuse = 1;
    dstrncpy(inodes[inode_handle].fname, filename, dstrlen(filename));
    // Release the lock
    while(LockHandleRelease(lock_inodes) != SYNC_SUCCESS);
    return inode_handle;
}

// DfsInodeDelete =========================================
// De-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary.
// Also, including the double indirect addressing block,
// if necesarry. Then mark the inode as no longer inuse.
// Use locks when modifying the inuse flag in an inode. 
// Return DFS_FAIL on failure and DFS_SUCCESS on good. 
// ========================================================
int DfsInodeDelete(uint32 handle) 
{
    // Initialize variables and parameters
    int i=0;

    // Check that filesystem is open
    if(sb.valid != 1 || dfsOpen != 1) return DFS_FAIL;

    // Let's grab the lock
    while(LockHandleAcquire(lock_inodes) != SYNC_SUCCESS);
    inodes[handle].fsize = 0;
    inodes[handle].inuse = 0;
    inodes[handle].fname[0] = '\0';
    for(i=0; i<DFS_INODE_BTABLE_SIZE; i++) inodes[handle].btable[i] = -1;
    if(inodes[handle].ibtable != -1) 
    {
        DfsFreeBlock(inodes[handle].ibtable);
        inodes[handle].ibtable = -1; 
    }
    if(inodes[handle].iibtable != -1) inodes[handle].iibtable = -1; 

    // Release the lock
    while(LockHandleRelease(lock_inodes) != SYNC_SUCCESS);
    return DFS_SUCCESS;
}

// DfsInodeReadBytes ======================================
// Reads num_bytes from the file represented by the inode
// handle, starting at virtual byte start_byte, copying
// the data to the address pointed to by mem. Return 
// DFS_FAIL on failure, and the number of bytes read on 
// success.
// ========================================================
int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) 
{
    // Initialize variables and parameters
    int dfsblocknum=0, read_bytes=0;
    int cpos = start_byte % sb.bsize;
    int vblocknum = start_byte / sb.bsize;
    dfs_block btable_buffer;
    char * ptr = mem;
    int * ibt;
    
    // Check that filesystem is open
    if(sb.valid != 1 || dfsOpen != 1) return DFS_FAIL;

    // Check if this filename exists
    if(inodes[handle].inuse != 1) return DFS_FAIL;

    while(read_bytes < num_bytes)
    {
        if(vblocknum < 10) dfsblocknum = inodes[handle].btable[vblocknum];
        else
        {
            if(inodes[handle].ibtable == -1) inodes[handle].ibtable = DfsAllocateBlock();
            if(DfsReadBlock(inodes[handle].ibtable, &btable_buffer) != sb.bsize) return DFS_FAIL;
            ibt = (int *)btable_buffer.data;
            dfsblocknum = ibt[vblocknum-10];
        }
        if(DfsReadBlock(dfsblocknum, &btable_buffer) != sb.bsize) return DFS_FAIL;

        if((num_bytes - read_bytes) > (sb.bsize - cpos))
        {
            bcopy(btable_buffer.data + cpos, ptr, sb.bsize - cpos);
            ptr += sb.bsize - cpos;
            read_bytes += sb.bsize - cpos;
            cpos = 0;
            vblocknum++;
        }
        else
        {
            bcopy(btable_buffer.data + cpos, ptr, num_bytes - read_bytes);
            read_bytes += sb.bsize - cpos;
            return read_bytes;
        }
    }
    return DFS_FAIL;
}

// DfsInodeWriteBytes =====================================
// Writes num_bytes from the memory pointed to by mem to 
// the file represented by the inode handle, starting at 
// virtual byte start_byte. Note that if you are only 
// writing part of a given file system block, you'll need 
// to read that block from the disk first. Return DFS_FAIL 
// on failure and the number of bytes written on success.
// ========================================================
int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes) 
{
    // Initialize variables and parameters
    int dfsblocknum=0, written_bytes=0;
    int cpos = start_byte % sb.bsize;
    int wblocknum = start_byte / sb.bsize;
    dfs_block btable_buffer;
    char * ptr = mem;
    int * ibt = NULL;
    
    // Check that filesystem is open
    if(sb.valid != 1 || dfsOpen != 1) return DFS_FAIL;

    // Check if this filename exists
    if(inodes[handle].inuse != 1) return DFS_FAIL;

    while(written_bytes < num_bytes)
    {
        if(wblocknum < 10) 
        {
            dfsblocknum = inodes[handle].btable[wblocknum];
            if(dfsblocknum == -1)
            {
                dfsblocknum=DfsInodeAllocateVirtualBlock(handle,wblocknum);
                if(dfsblocknum==DFS_FAIL) return DFS_FAIL;
            }
        }
        else
        {
            if(ibt == NULL)
            {
                if(inodes[handle].ibtable == -1) 
                {
                    dfsblocknum = DfsInodeAllocateVirtualBlock(handle, wblocknum);
                }
                if(DfsReadBlock(inodes[handle].ibtable, &btable_buffer) != sb.bsize) return DFS_FAIL;
                ibt = (int *)btable_buffer.data;
            }
            dfsblocknum = ibt[wblocknum - 10];
            if(dfsblocknum == -1) 
            {
                dfsblocknum = DfsInodeAllocateVirtualBlock(handle, wblocknum);
                if(dfsblocknum==DFS_FAIL) return DFS_FAIL;
            }
        }
        if(DfsReadBlock(dfsblocknum, &btable_buffer) != sb.bsize) return DFS_FAIL;

        if((num_bytes - written_bytes) > (sb.bsize - cpos))
        {
            bcopy(ptr, btable_buffer.data + cpos, sb.bsize - cpos);
            if(DfsWriteBlock(dfsblocknum, &btable_buffer) != sb.bsize) return DFS_FAIL;
            ptr += sb.bsize - cpos;
            written_bytes += sb.bsize - cpos;
            cpos = 0;
            wblocknum++;
        }
        else
        {
            bcopy(ptr, btable_buffer.data + cpos, num_bytes - written_bytes);
            if(DfsWriteBlock(dfsblocknum, &btable_buffer) != sb.bsize) return DFS_FAIL;
            written_bytes += num_bytes - written_bytes;
            if(start_byte + num_bytes > inodes[handle].fsize)
            { inodes[handle].fsize = start_byte + num_bytes;  }
            return written_bytes;
        }
    }
    return DFS_FAIL;
}

// DfsInodeFilesize =======================================
// Simply returns the size of an inode's file. This is 
// defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on 
// failure.
// ========================================================
uint32 DfsInodeFilesize(uint32 handle) 
{
    // Check that filesystem is open
    if(sb.valid != 1 || dfsOpen != 1) return DFS_FAIL;

    // Check if this filename exists
    if(inodes[handle].inuse != 1) return DFS_FAIL;
    return inodes[handle].fsize;
}

// DfsInodeAllocateVirtualBlock ===========================
// Allocates a new filesystem block for the given inode, 
// storing its blocknumber at index virtual_blocknumber in 
// the translation table. If the virtual_blocknumber 
// resides in the indirect address space, and there is not 
// an allocated indirect addressing table, allocate it. 
// Return DFS_FAIL on failure, and the newly allocated file 
// system block number on success.
// ========================================================
uint32 DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) 
{
    // Initialize variables and parameters
    int dfsblocknum=0;
    dfs_block dfsblock_buffer;
    int * ibt = NULL;
    
    // Check that filesystem is open
    if(sb.valid != 1 || dfsOpen != 1) return DFS_FAIL;

    // Check if this filename exists
    if(inodes[handle].inuse != 1) return DFS_FAIL;

    if(virtual_blocknum < 0 || virtual_blocknum >= 10 + sb.bsize/4) return DFS_FAIL;
    if(virtual_blocknum < 10) 
    {
        if(inodes[handle].btable[virtual_blocknum] != -1) return DFS_FAIL;
        dfsblocknum = DfsAllocateBlock();
        inodes[handle].btable[virtual_blocknum] = dfsblocknum;
    }
    else
    {
        if(inodes[handle].ibtable == -1) 
        {  inodes[handle].ibtable = DfsAllocateBlock();  }
        if(DfsReadBlock(inodes[handle].ibtable, &dfsblock_buffer) != sb.bsize) return DFS_FAIL;
        ibt = (int *)dfsblock_buffer.data;
        dfsblocknum = DfsAllocateBlock();
        ibt[virtual_blocknum-10] = dfsblocknum;
        if(DfsWriteBlock(inodes[handle].ibtable, &dfsblock_buffer) != sb.bsize) return DFS_FAIL;
    }
    return dfsblocknum; 
}

// DfsInodeTranslateVirtualToFilesys ======================
// Translates the virtual_blocknum to the corresponding 
// file system block using the inode identified by handle. 
// Return DFS_FAIL on failure.
// ========================================================
uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum) 
{
    // Initialize variables and parameters
    int dfsblocknum=0;
    dfs_block dfsblock_buffer;
    int * ibt;
    
    // Check that filesystem is open
    if(sb.valid != 1 || dfsOpen != 1) return DFS_FAIL;

    // Check if this filename exists
    if(inodes[handle].inuse != 1) return DFS_FAIL;

    if(virtual_blocknum < 10) 
    {
        if(inodes[handle].btable[virtual_blocknum] != -1) return DFS_FAIL;
        dfsblocknum = inodes[handle].btable[virtual_blocknum];
    }
    else
    {
        if(inodes[handle].ibtable == -1) return DFS_FAIL;
        if(DfsReadBlock(inodes[handle].ibtable, &dfsblock_buffer) != sb.bsize) return DFS_FAIL;
        ibt = (int *)dfsblock_buffer.data;
        dfsblocknum = ibt[virtual_blocknum-10];
    }
    return dfsblocknum; 
}
