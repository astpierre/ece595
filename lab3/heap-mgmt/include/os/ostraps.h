#ifndef __stdio_os__
#define __stdio_os__

//-----------------------------------------------------------------------
// Functions declarations in this file are the various os-level traps
// defined in the main simulator library at:
// ~ee469/labs/common/dlxos/lib/gcc-lib/dlx/2.7.2.3/libtraps.a
// These declarations are necessary to prevent the compiler from 
// throwing errors.
//-----------------------------------------------------------------------

void printf(const char *fmt, ...);
int open(const char *name, int mode);
int read(int fd, char *buf, int numbytes);
int write(int fd, char *buf, int numbytes);
int lseek(int fd, int offset, int where);
int close(int fd);
void bcopy(char *source, char *destination, int numbytes);
void exitsim();
void TimerSet(int us);


#endif
