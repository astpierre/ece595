//
//	filesys.c
//
//	This file has handlers for the file systems supported by the DLX
//	operating system.  These include both the basic, simulator-provided
//	file system and the more complex file system from the file system
//	assignment.

//static char rcsid[] = "$Id: filesys.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "filesys.h"

// One entry for each type of file system.  Currently, the file systems
// are native Unix (0) and DLX (1).
static Fs	fs[2];

// One entry for each open file in the operating system.
static FsOpenFile openfiles[FS_MAX_OPEN_FILES];

//----------------------------------------------------------------------
//
//	FdValid
//
//	Return 1 if the file descriptor is valid, 0 otherwise.
//
//----------------------------------------------------------------------
static
inline
int
FdValid (int fd)
{
  return ((fd > 0) && (fd < FS_MAX_OPEN_FILES) && (openfiles[fd].flags != 0));
}

//----------------------------------------------------------------------
//
//	FsFreeEntry
//
//	Free an entry in the open files table.
//
//----------------------------------------------------------------------
static
inline
void
FsFreeEntry (int x)
{
  openfiles[x].flags = 0;
}


//----------------------------------------------------------------------
//
//	FsOpen
//
//	Open a file.  The name of the file is passed, along with the file
//	mode (read or write).
//
//	This function figures out which file system is desired using a
//	simple heuristic.  Basically, if the file starts with "dlx:", it's
//	a DLX file system file.  Otherwise, it's a UNIX file.
//
//	Once the file system is figured out, this routine allocates a
//	file descriptor and calls the file-system specific open routine.
//
//----------------------------------------------------------------------
int
FsOpen (const char *name, int mode)
{
  int		i, retval;

  dbprintf ('f', "Attepmting to open %s mode=%d.\n", name, mode);
  // Mask off all but the mode bits
  mode &= FS_MODE_RW;
  // ERROR if the caller hasn't specified a file mode.
  if (mode == 0) {
    return (-1);
  }
  for (i = 0; i < FS_MAX_OPEN_FILES; i++) {
    if (openfiles[i].flags == 0) {
      break;
    }
  }
  if (i >= FS_MAX_OPEN_FILES) {
    return (-1);
  }
  openfiles[i].flags = mode;
  // If file name starts with "dlx:", it's a DLX file system file.  Pass
  // the remainder of the file name to FsDlxOpen.
  if (!dstrncmp (name, "dlx:", 4)) {
    name += 4;			// Skip past the "dlx:"
    openfiles[i].fs = 1;
  } else {
    openfiles[i].fs = 0;
  }
  dbprintf ('f', "File %s opening in file system %d.\n",name,openfiles[i].fs);
  retval = fs[openfiles[i].fs].Open (i, name, mode);
  if (retval < 0) {
    // Open failed, so return error code
    FsFreeEntry (i);
    return (retval);
  }
  dbprintf ('f', "Opened %s in FS %d, mode=%d slot=%d.\n", name,
	    openfiles[i].fs, mode, i);
  return (i);
}

//----------------------------------------------------------------------
//
//	FsClose
//
//	Close an open file.  Call the FS-specific routine, and then mark
//	the file table entry unused.
//
//----------------------------------------------------------------------
int
FsClose (int fd)
{
  int	retval;

  if (!FdValid (fd)) {
    return (-1);
  }
  retval = fs[openfiles[fd].fs].Close (fd);
  FsFreeEntry (fd);
  return (retval);
}


//----------------------------------------------------------------------
//
//	FsRead
//
//	Read data from a file.
//
//----------------------------------------------------------------------
int
FsRead (int fd, char *buf, int n)
{
  if (!FdValid (fd)) {
    return (-1);
  }
  return (fs[openfiles[fd].fs].Read (fd, buf, n));
}

//----------------------------------------------------------------------
//
//	FsWrite
//
//	Write data to a file.
//
//----------------------------------------------------------------------
int
FsWrite (int fd, char *buf, int n)
{
  if (!FdValid (fd)) {
    return (-1);
  }
  return (fs[openfiles[fd].fs].Write (fd, buf, n));
}

//----------------------------------------------------------------------
//
//	FsSeek
//
//	Seek in a file.
//
//----------------------------------------------------------------------
int
FsSeek (int fd, int offset, int whence)
{
  if (!FdValid (fd)) {
    return (-1);
  }
  return (fs[openfiles[fd].fs].Seek (fd, offset, whence));
}


//----------------------------------------------------------------------
//
//	FsDelete
//
//	Delete a file.
//
//----------------------------------------------------------------------
int
FsDelete (const char *name)
{
  // If file name starts with "dlx:", it's a DLX file system file.  Pass
  // the remainder of the file name to FsDlxDelete.
  if (!dstrncmp (name, "dlx:", 4)) {
    name += 4;			// Skip past the "dlx:"
    return (fs[1].Delete (name));
  } else {
    return (fs[0].Delete (name));
  }
}


//----------------------------------------------------------------------
//
//	FsUnixOpen
//	FsUnixRead
//	FsUnixWrite
//	FsUnixSeek
//	FsUnixClose
//
//	Unix file I/O routines.  These are pretty simple, and just call
//	the simulator traps directly using the file descriptor stored in
//	the open file table.
//
//----------------------------------------------------------------------
int
FsUnixOpen (int x, const char *name, int mode)
{
  dbprintf ('f', "Attempting to open file %s into desc %d, mode %d.\n",
	    name, x, mode);
  if ((openfiles[x].u.Unix.fd = open (name, mode)) < 0) {
    return (openfiles[x].u.Unix.fd);
  }
  return (1);
}

int
FsUnixRead (int x, char *buf, int n)
{
  return (read (openfiles[x].u.Unix.fd, buf, n));
}

int
FsUnixWrite (int x, char *buf, int n)
{
  return (write (openfiles[x].u.Unix.fd, buf, n));
}

int
FsUnixSeek (int x, int offset, int where)
{
  return (lseek (openfiles[x].u.Unix.fd, offset, where));
}

int
FsUnixClose (int x)
{
  int	retval;

  retval = close (openfiles[x].u.Unix.fd);
  return (retval);
}

int
FsUnixDelete (const char *name)
{
  // Refuse to delete Unix files via the simulator.  This is for safety
  // reasons.
  return (-1);
}


//----------------------------------------------------------------------
//
//	FsDlxIo
//
//	Perform a read or a write on a DLX file.
//
//----------------------------------------------------------------------
int
FsDlxIo (int fd, char *buf, int n, int which)
{
  // Your code to read or write a DLX file goes here.
  return (1);
}


//----------------------------------------------------------------------
//
//	FsDlxRead
//	FsDlxWrite
//
//	This is just a stub that calls FsDlxIo to do either a read or a
//	write in the DLX file system.
//
//----------------------------------------------------------------------
int
FsDlxRead (int fd, char *buf, int n)
{
  return (FsDlxIo (fd, buf, n, 0));
}

int
FsDlxWrite (int fd, char *buf, int n)
{
  return (FsDlxIo (fd, buf, n, 1));
}

//----------------------------------------------------------------------
//
//	FsDlxOpen
//
//	Open a file in the DLX file system.  Currently, this just means
//	to set current position to 0.  However, it should also check to
//	make sure that the file exists.
//
//----------------------------------------------------------------------
int
FsDlxOpen (int f, const char *name, int mode)
{
  openfiles[f].u.Dlx.curpos = 0;
  // You may want to put additional code here (checking to see if the
  // file exists, etc.)
  return (1);
}

//----------------------------------------------------------------------
//
//	FsDlxSeek
//
//	Seek in a DLX file.  This just requires adjusting the current
//	position as recorded in the open files table.
//
//----------------------------------------------------------------------
int
FsDlxSeek (int f, int offset, int whence)
{
  switch (whence) {
  case FS_SEEK_SET:
    break;
  case FS_SEEK_CUR:
    break;
  case FS_SEEK_END:
    break;
  };
  return (openfiles[f].u.Dlx.curpos);
}

//----------------------------------------------------------------------
//
//	FsDlxClose
//
//	Close a DLX file system file.
//----------------------------------------------------------------------
int
FsDlxClose (int f)
{
  // You may want to add code to this routine.
  return (1);
}

//----------------------------------------------------------------------
//
//	FsDlxDelete
//
//	Delete a file in the DLX file system.
//
//----------------------------------------------------------------------
int
FsDlxDelete (const char *file)
{
  // Your code goes here.
  return (1);
}


//----------------------------------------------------------------------
//
//	FsModuleInit
//
//	Initialize the file system module.  This consists simply of marking
//	all the file slots as available.
//
//----------------------------------------------------------------------
void
FsModuleInit ()
{
  int	i;

  for (i = 0; i < FS_MAX_OPEN_FILES; i++) {
    openfiles[i].flags = 0;
  }

  fs[0].Open = FsUnixOpen;
  fs[0].Close = FsUnixClose;
  fs[0].Read = FsUnixRead;
  fs[0].Write = FsUnixWrite;
  fs[0].Seek = FsUnixSeek;
  fs[0].Delete = FsUnixDelete;

  fs[1].Open = FsDlxOpen;
  fs[1].Close = FsDlxClose;
  fs[1].Read = FsDlxRead;
  fs[1].Write = FsDlxWrite;
  fs[1].Seek = FsDlxSeek;
  fs[1].Delete = FsDlxDelete;
}
