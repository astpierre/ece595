/* A simple usage of FS APIs
*/


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <unistd.h>

int main (int argc, char *argv[])
{
  int fd1, fd2, fd3, pid, i, j;
  char string[11]="0123456789";
  char stringout[11];

 if ((fd1 = open ("foo", O_RDONLY|O_WRONLY)) < 0)
   perror ("1st time: can't open %s for reading/writing", "foo");
 if ((fd2 = open ("bar", O_RDONLY|O_WRONLY)) < 0)
   perror ("1st time: can't open %s for reading/writing", "foo");
 printf("1st time: fd1 = %d fd2 = %d \n", fd1, fd2);
 close(fd1);
 close(fd2);

 if ((fd1 = open ("foo", O_RDONLY|O_WRONLY)) < 0)
   perror ("2nd time: can't open %s for reading/writing", "foo");
 if ((fd2 = open ("bar", O_RDONLY|O_WRONLY)) < 0)
   perror ("2nd time: can't open %s for reading/writing", "foo");
 printf("2nd time: fd1 = %d fd2 = %d\n", fd1, fd2);

 for (i=0; i<2; i++)
   write(fd1, string, 10);
 close(fd1);


 if ((fd1 = open ("foo", O_RDONLY|O_WRONLY)) < 0)
   perror ("2nd time: can't open %s for reading/writing", "foo");
 if ((fd2 = open ("bar", O_RDONLY|O_WRONLY)) < 0)
   perror ("2nd time: can't open %s for reading/writing", "foo");
 printf("3rd time: fd1 = %d fd2 = %d\n", fd1, fd2);

 close(fd1);
 close(fd2);

 exit(0);
} 
