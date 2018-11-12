#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "filesys.h"

//----------------------------------------------------------------------------
// DiskBytesPerBlock returns the number of bytes in each physical block
// on the disk.
//----------------------------------------------------------------------------

int DiskBytesPerBlock() {
  return DISK_BLOCKSIZE;
}

//----------------------------------------------------------------------------
// DiskSize returns the size of the hard disk, in bytes.
//----------------------------------------------------------------------------

int DiskSize() {
  return DISK_BLOCKSIZE * DISK_NUMBLOCKS;
}

//----------------------------------------------------------------------------
// DiskCreate opens the filesystem for writing, which will erase whatever
// was there before.  You need to call this only when formattig the
// disk to make sure that the file actually exists.
//----------------------------------------------------------------------------

int DiskCreate() {
  char *filename = NULL;
  int fsfd = -1;
  disk_block b;
  int i;

  // Check that you remembered to rename the filename for your group
  filename = DISK_FILENAME;
  if (filename[11] == 'X') {
    printf("DiskCreate: you didn't change the filesystem filename in include/os/disk.h.  Cowardly refusing to do anything.\n");
    GracefulExit();
  }
  // Open the hard disk file
  if ((fsfd = FsOpen(DISK_FILENAME, FS_MODE_WRITE)) < 0) {
    printf ("DiskCreate: File system %s cannot be opened!\n", DISK_FILENAME);
    return DISK_FAIL;
  }

  // Write all zeros to the hard disk file to make sure it is the right size.
  // You need to do this because the writeblock/readblock operations are allowed in
  // random order.
  bzero(b.data, DISK_BLOCKSIZE);
  for(i=0; i<DISK_NUMBLOCKS; i++) {
    FsWrite(fsfd, b.data, DISK_BLOCKSIZE);
  }
  
  // Close the hard disk file
  if (FsClose(fsfd) < 0) {
    printf("DiskCreate: unable to close open file!\n");
    return DISK_FAIL;
  }
  return DISK_SUCCESS;
}

//----------------------------------------------------------------------------
// DiskWriteBlock writes one block to the disk, using the bytes pointed to 
// by memory.  The blocksize is specified by DISK_BLOCKSIZE.  Returns
// the number of bytes written on success, or DISK_FAIL on failure.
//----------------------------------------------------------------------------

int DiskWriteBlock (uint32 blocknum, disk_block *b) {
  int fsfd = -1;
  uint32 intrvals = 0;
  char *filename = NULL;

  if (blocknum >= DISK_NUMBLOCKS) {
    printf("DiskWriteBlock: cannot write to block larger than filesystem size\n");
    return DISK_FAIL;
  }

  // Check that you remembered to rename the filename for your group
  filename = DISK_FILENAME;
  if (filename[11] == 'X') {
    printf("DiskWriteBlock: you didn't change the filesystem filename in include/os/disk.h.  Cowardly refusing to do anything.\n");
    GracefulExit();
  }

  intrvals = DisableIntrs();

  // Open the hard disk file
  if ((fsfd = FsOpen(DISK_FILENAME, FS_MODE_RW)) < 0) {
    printf ("DiskWriteBlock: File system %s cannot be opened!\n", DISK_FILENAME);
    return DISK_FAIL;
  }

  // Write data to virtual disk
  FsSeek(fsfd, blocknum * DISK_BLOCKSIZE, FS_SEEK_SET);
  if (FsWrite(fsfd, b->data, DISK_BLOCKSIZE) != DISK_BLOCKSIZE) {
    printf ("DiskWriteBlock: Block %d could not be written!\n", blocknum);
    FsClose (fsfd);
    return DISK_FAIL;
  }

  // Close the hard disk file
  FsClose (fsfd);

  RestoreIntrs(intrvals);
  return DISK_BLOCKSIZE;
}

//----------------------------------------------------------------------------
// DiskReadBlock reads one block from the disk, putting the bytes into the
// memory pointed to by memory.  The blocksize is specified by DISK_BLOCKSIZE.
// Returns the number of bytes read on success, or DISK_FAIL on failure.
//----------------------------------------------------------------------------

int DiskReadBlock (uint32 blocknum, disk_block *b) {
  int fsfd = -1;
  uint32 intrvals = 0;
  char *filename = NULL;

  if (blocknum >= DISK_NUMBLOCKS) {
    printf("DiskReadBlock: cannot read from block larger than filesystem size\n");
    return DISK_FAIL;
  }

  if (filename[11] == 'X') {
    printf("DiskReadBlock: you didn't change the filesystem filename in include/os/disk.h.  Cowardly refusing to do anything.\n");
    GracefulExit();
  }

  intrvals = DisableIntrs();

  // Open the hard disk file
  if ((fsfd = FsOpen(DISK_FILENAME, FS_MODE_READ)) < 0) {
    printf ("DiskReadBlock: File system %s cannot be opened!\n", DISK_FILENAME);
    return DISK_FAIL;
  }

  // Read data from virtual disk
  FsSeek(fsfd, blocknum * DISK_BLOCKSIZE, FS_SEEK_SET);
  if (FsRead(fsfd, b->data, DISK_BLOCKSIZE) != DISK_BLOCKSIZE) {
    printf ("DiskReadBlock: Block %d could not be read!\n", blocknum);
    FsClose (fsfd);
    return DISK_FAIL;
  }

  // Close the hard disk file
  FsClose (fsfd);

  RestoreIntrs(intrvals);
  return DISK_BLOCKSIZE;
}


