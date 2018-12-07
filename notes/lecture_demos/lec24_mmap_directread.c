/*
  An example showing two ways of reading a file:
  1. via normal read() system call
  2. mmap the file, followed by memory reads to the mapped address region
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define SIZE 10
main()
{
  char *ptr1, *ptr3;
  char ptr2[SIZE];
  unsigned int count = SIZE;
  int i, ii, fd;
  FILE *fp;

  fp = fopen("foo", "w");
  for (i=0; i<(count); i++)
    fprintf(fp, "%d", i);
  fclose(fp);

  printf("read the file the old-fashioned way:\n");
  fp = fopen("foo", "r");
  for (i=0; i<(count); i++) {
    fread(ptr2, sizeof(char), 1, fp);   printf("ii = %c\n",  *ptr2);
  }
  fclose(fp);
 
  printf("read the file the fancy way:\n"); 
  if ((fd = open("foo", O_RDWR)) == -1)
    printf("failure in opening foo!\n");
  if ((ptr1 = (char *) mmap(0, count, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0))
      == MAP_FAILED)
    printf("could not mmap %u bytes -- errno = %d \n", (unsigned int) count, errno);
  else {
    printf("mmaped %u bytes at %p to %p\n", (unsigned int) count, 
	   ptr1, ptr1 + (unsigned int) count);
  }

  for (i=0; i<count; i++) 
    printf("ii = %c\n", *(ptr1+i));

  exit(0);
}

