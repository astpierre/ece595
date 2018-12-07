/*
 * this user program tries to read/write directly from/to a dir
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define SIZE 11
main()
{
  char ptr2[SIZE];
  char ptr1[20]; 
  unsigned int count = SIZE;
  int i, ii;
  FILE *fd;

  strcpy(ptr1, "september");

  if ((fd = fopen("foo2", "r")) == NULL) 
    printf("failure in open dir foo2 for read !\n");
  else {
    printf("Opned foo2 for reading....\n");
    for (i=0; i<(count); i++) {
      fread(ptr2, sizeof(char), 1, fd);   
      printf("ii = %c\n",  *ptr2);
    }
    fclose(fd);
  }

  if ((fd = fopen("foo2", "w")) == NULL)
    printf("failure in opening dir foo2 for write !\n");
  else {
    printf("Opened dir...writing dir foo2\n");
  
    for (i=0; i<(count); i++)
      fwrite(ptr1, sizeof(char), 1, fd);
    fclose(fd);
  }

  exit(0);
}
