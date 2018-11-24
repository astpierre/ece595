#ifndef __FILES_SHARED__
#define __FILES_SHARED__

/* Define variables used by the file system (not DFS) here */
#define FILE_SEEK_SET 1
#define FILE_SEEK_END 2
#define FILE_SEEK_CUR 3

#define FILE_MAX_FILENAME_LENGTH 72 
#define FILE_MAX_READWRITE_BYTES 1024 

typedef struct file_descriptor {
    uint32 inuse;
    char file_name[FILE_MAX_FILENAME_LENGTH];
    uint32 inode;
    uint32 eof;
    uint32 current_position;
    uint32 pid;
    char mode;
} file_descriptor;

#define FILE_FAIL -1
#define FILE_EOF -1
#define FILE_SUCCESS 1

#endif
