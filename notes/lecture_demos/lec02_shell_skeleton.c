#include <stdio.h>

/*
 * use fork() and execlp() to implement a toy "shell",
 * ignoring the X window stuff of course
 *
 * example usage of execlp():
 *
 *     execlp("/bin/ls", "", NULL);
 *
 */

main()
{
  int pid;
  char a[32];

  while (1) {
    scanf("%s", a);
    printf(">>>>>>>>>>>>>>>>> the input command is %s\n", a);

    /* fill in the missing code here */















  }

  exit(0);
}
