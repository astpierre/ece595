//
// filesys.h
//
// Includes for the file system.  Includes the definition of the file
// system interface structure.
//
// Copyright 1999 by Ethan L. Miller
// University of Maryland Baltimore County
//
// $Id: filesys.h,v 1.1 2000/09/20 01:50:19 elm Exp elm $
//

#ifndef	_filesys_h_
#define	_filesys_h_

#define	FS_FS_BUILTIN	1
#define	FS_FS_DLXOS	2

#define	FS_MODE_READ	0x1
#define	FS_MODE_WRITE	0x2
#define	FS_MODE_RW	(FS_MODE_READ | FS_MODE_WRITE)
// If this flag is set, a new file is created if no file already exists.
// This flag only works with the DLX file system.
#define	FS_MODE_CREATE	0x4

// IMPORTANT: These must match the underlying UNIX seek parameters.
// These values are correct for Linux and should work elsewhere, but
// be careful....
#define	FS_SEEK_SET	0
#define	FS_SEEK_CUR	1
#define	FS_SEEK_END	2

#define	FS_BUFFER_SIZE	256

#define	FS_MAX_OPEN_FILES 32

typedef struct Fs {
  int	(*Open)(int, const char *, int);
  int	(*Read)(int, char *, int);
  int	(*Write)(int, char *, int);
  int	(*Seek)(int, int, int);
  int	(*Close)(int);
  int	(*Delete)(const char *);
} Fs;

typedef struct FsUnixInfo {
  int	fd;		// Unix file descriptor
} FsUnixInfo;

typedef struct FsDlxInfo {
  int	curpos;		// What's our current position in the file?
} FsDlxInfo;

typedef struct FsOpenFile {
  int	fs;		// which file system is this file in?
  int	flags;
  char	buffer[FS_BUFFER_SIZE];
  union {
    FsUnixInfo	Unix;
    FsDlxInfo	Dlx;
  } u;
} FsOpenFile;

extern void FsModuleInit();
extern int	FsOpen (const char *, int);
extern int	FsRead (int, char *, int);
extern int	FsWrite (int, char *, int);
extern int	FsSeek (int, int, int);
extern int	FsClose (int);
extern int	FsDelete (const char *);

#endif	// _filesys_h_
