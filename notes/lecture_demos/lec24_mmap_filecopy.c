/*
Here is the example of using mmap() to implement file copy (e.g., UNIX cp).

Two different processes can also map the same file, for example, and
use this as a form of inter-process communication (but making sure to
control concurrency, e.g., using semaphores).

Note how we write a byte at the last location (statbuf.st_size - 1) of
the output file here to extend its size before mmap()ing the output
file. This (dummy) byte will be overwritten when we memcpy().
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <unistd.h>

int main (int argc, char *argv[])
{
 int fdin, fdout, i;
 char *src, *dst;
 struct stat statbuf;

 if (argc != 3) { perror("usage: a.out <fromfile> <tofile>"); exit(1);}

 /* open the input file */
 if ((fdin = open (argv[1], O_RDONLY)) < 0)
   perror ("can't open %s for reading", argv[1]);

 /* open/create the output file */
  if ((fdout = open (argv[2], O_RDWR | O_CREAT | O_TRUNC,  S_IRWXU)) < 0)
   perror ("can't create %s for writing", argv[2]);

 /* find size of input file */
 if (fstat (fdin,&statbuf) < 0)
   perror ("fstat error");

 /* go to the location corresponding to the last byte */
 if (lseek (fdout, statbuf.st_size - 1, SEEK_SET) == -1)
   perror ("lseek error");
 
 /* write a dummy byte at the last location */
  if (write (fdout, "", 1) != 1) perror ("write error");

 /* mmap the input file */
 if ((src = mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0))
   == (caddr_t) -1)
   perror ("mmap error for input");

 /* read the stuff out like memory reference! */
 for (i=0; i<statbuf.st_size-1; i++)
   printf("direct memory read: scr = %c\n", *(src+i) );
 
 /* mmap the output file */
 if ((dst = mmap (0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0))
     == (caddr_t) -1)
   perror ("mmap error for output");

 /* this copies the input file to the output file */
 printf("executing 'memcpy (dst, src, statbuf.st_size);' \n");
 memcpy (dst, src, statbuf.st_size);

 exit(0);
} 










