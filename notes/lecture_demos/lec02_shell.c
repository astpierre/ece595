#include <stdio.h>

main()
{
  int pid;
  char a[32];

  while (1) {
    scanf("%s", a);
    printf(">>>>>>>>>>>>>>>>> input command is %s\n", a);

    pid = fork();

    if (pid < 0) {printf("Fork failed\n"); exit(1); }
    else {
      if (pid == 0) {
	execlp(a, "", NULL);
	exit(0);
      } else {
	printf("Parent process: Child pid is %d\n", pid);
	wait();
      }
    }
  }
}
