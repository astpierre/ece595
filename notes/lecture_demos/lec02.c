#include <stdio.h>
#include <unistd.h>

int myval;

int main(int argc, char *argv[])
{
  myval = atoi(argv[1]);
  while (1){
    sleep (1);
    printf("myval = %d, loc = 0x%lx\n",
           myval, (long) &myval);
  }
  //  exit(0);
}

