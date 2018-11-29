#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

// Global declarations
static file_descriptor files[DFS_INODE_NMAX_NUM]; // 128 files
//static lock_t lock;
static int openFiles = 0;

int getModeNum(char * mode)
{
    if((mode[0] == 'r') || (mode[0] == 'R')) return FMODE_R;
    if((mode[0] == 'w') || (mode[0] == 'W')) return FMODE_W;
    else return FILE_FAIL;
}

int getFileHandle(int inodehandle)
{
    // Variable declarations
    int i=0;
    // Scan files for matching inode handles
    for(i=0; i<DFS_INODE_NMAX_NUM; i++)
    {  if(files[i].inodeHandle == inodehandle) return i;  }
    return FILE_FAIL;
}

int getAnotherFileDescriptor()
{
    // Variable declarations
    int i=0;
    // Scan files for free inode handles
    for(i=0; i<DFS_INODE_NMAX_NUM; i++)
    {  if(files[i].inodeHandle == -1) return i;  }
    return FILE_FAIL;
}

uint32 FileOpen(char * filename, char * mode) 
{
    // Variable declarations
    int m, inodehandle, fhandle;

    // Check that MAX_OPEN_FILES < 15
    if(openFiles >= FILE_MAX_OPEN_FILES) return -1;

    // Check that filename is not open in another file_descriptor
    inodehandle = DfsInodeFilenameExists(filename);
    if(inodehandle != DFS_FAIL)
    {
        fhandle = getFileHandle(inodehandle);
        // File is being used by another process! Don't open this file!
        if((fhandle != FILE_FAIL) && (GetCurrentPid() != files[fhandle].mypid)) return FILE_FAIL;
    }

    // Use helper function to convert mode to number
    if((m = getModeNum(mode)) == FILE_FAIL) return FILE_FAIL;
    
    // If we are in W mode, we want to do the following...
    if(m == FMODE_W)
    {
        // 1. If the file already exists
        if(inodehandle != DFS_FAIL)
        {
            // 2. Delete the current inode
            if(DfsInodeDelete(inodehandle) != DFS_SUCCESS) return FILE_FAIL;
        }
        // 3. Reopen inode
        inodehandle = DfsInodeOpen(filename);
    }

    // If file is nonexistent, handle this
    if(inodehandle == DFS_FAIL) return FILE_FAIL;

    // Grab the lock, we are going to alter files structure
    //while(LockHandleAcquire(lock) != SYNC_SUCCESS);

    // Our fhandle was taken while waiting for lock or something, grab another
    if(fhandle == FILE_FAIL) fhandle = getAnotherFileDescriptor();

    // Assign values to data structure
    dstrncpy(files[fhandle].fname, filename, dstrlen(filename));
    files[fhandle].inodeHandle = inodehandle;
    files[fhandle].mypid = GetCurrentPid();
    openFiles += 1;

    // Release the lock, we return the new file handle
    //while(LockHandleRelease(lock) != SYNC_SUCCESS);
    return fhandle;
}

void CleanFileDescriptor(uint32 handle)
{
    bzero(files[handle].fname, FILE_MAX_FILENAME_LENGTH);
    files[handle].inodeHandle = -1;
    files[handle].mypid = -1;
    files[handle].cpos = -1;
    files[handle].eof = -1;
    files[handle].mode = '\0';
}

int FileClose(uint32 handle) 
{
    // Variable declarations
    // Check that calling process was the process who opened file
    if(files[handle].mypid != GetCurrentPid()) return FILE_FAIL;
    // Grab the lock, we are going to alter files structure
    //while(LockHandleAcquire(lock) != SYNC_SUCCESS);
    // Delete the file descriptor
    CleanFileDescriptor(handle);
    openFiles -= 1;
    // Release the lock, we return FILE_SUCCESS if nothing fails
    //while(LockHandleRelease(lock) != SYNC_SUCCESS);
    return FILE_SUCCESS;
}

int FileRead(uint32 handle, void * mem, int num_bytes) 
{
    // Variable declarations
    int bytes_read=0;
    // Check that calling process was the process who opened file
    if(files[handle].mypid != GetCurrentPid()) return FILE_FAIL;
    // Check that num_bytes > 0 && < FILE_MAX_READWRITE_BYTES
    if(num_bytes <= 0 || num_bytes > FILE_MAX_READWRITE_BYTES) return FILE_FAIL;
    // Perform reading
    bytes_read = DfsInodeReadBytes(files[handle].inodeHandle, mem, files[handle].cpos, num_bytes);
    if(bytes_read == DFS_FAIL) return FILE_FAIL;
    files[handle].cpos += bytes_read;
    if(bytes_read < num_bytes) {  files[handle].eof = 1; return FILE_FAIL;  }
    return bytes_read;
}

int FileWrite(uint32 handle, void * mem, int num_bytes) 
{
    // Variable declarations
    int bytes_written=0;
    // Check that calling process was the process who opened file
    if(files[handle].mypid != GetCurrentPid()) return FILE_FAIL;
    // Check that num_bytes > 0 && < FILE_MAX_READWRITE_BYTES
    if(num_bytes <= 0 || num_bytes > FILE_MAX_READWRITE_BYTES) return FILE_FAIL;
    // Perform reading
    bytes_written = DfsInodeWriteBytes(files[handle].inodeHandle, mem, files[handle].cpos, num_bytes);
    if(bytes_written == DFS_FAIL) return FILE_FAIL;
    return bytes_written;
}

int FileSeek(uint32 handle, int num_bytes, int from_where) 
{
    // Variable declarations

    // Check that calling process was the process who opened file
    if(files[handle].mypid != GetCurrentPid()) return FILE_FAIL;
 
    // Determinite what from_where is, and how to approach problem
    if(from_where == FILE_SEEK_SET) files[handle].cpos = num_bytes;
    else if(from_where == FILE_SEEK_END) files[handle].cpos = DfsInodeFilesize(files[handle].inodeHandle);
    else if(from_where == FILE_SEEK_CUR) files[handle].cpos += num_bytes;
    else return FILE_FAIL;

    // Check that current position is valid
    if(files[handle].cpos < 0 || files[handle].cpos > DfsInodeFilesize(files[handle].inodeHandle))
        return FILE_FAIL;

    files[handle].eof = 0;
    return FILE_SUCCESS;
}

int FileDelete(char *filename) 
{
    // Variable declarations
    int inodeh, fhndl=-1;

    // Check if opened by another proc
    inodeh = DfsInodeFilenameExists(filename);
    if(inodeh != DFS_FAIL) 
    {
        fhndl = getFileHandle(inodeh);
        if((fhndl != FILE_FAIL) & (GetCurrentPid() != files[fhndl].mypid)) return FILE_FAIL;
    }
    if(DfsInodeDelete(inodeh) != DFS_SUCCESS) return FILE_FAIL;
    if(fhndl != -1) 
    {
        // Grab the lock, we are going to alter files structure
        //while(LockHandleAcquire(lock) != SYNC_SUCCESS);
        // Delete the file descriptor
        CleanFileDescriptor(fhndl);
        // Release the lock, we return 0 if nothing fails
        //while(LockHandleRelease(lock) != SYNC_SUCCESS);
    }
    return 0;
}
