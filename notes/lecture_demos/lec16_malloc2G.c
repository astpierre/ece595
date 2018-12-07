/*
 * malloc 1.5G on a PC with 512MB DRAM
 */

#include <fcntl.h>
#include <stdio.h>
#include <time.h>
 
#define SIZE 1024*1020*1024

int main()
{
  int *pt, i;

  sleep(5);
  printf("before mallocing %d MB on a machine with 1024MB (ng16) -- you have 5 secs to evacuate!\n\n",(SIZE)>>20);

  sleep(5);

  pt = (int *) malloc(SIZE);
  printf("Allocated %d MB at %p -- This the day The Earth Stood Still !                                \n", SIZE>>20, pt);
  printf("How many physical pages have been allocatd -- 1 ????????\n");

  sleep(10);
  
  for(i=0; i<SIZE>>2; i++){
    if (i%(1<<27) == 0) 
       printf("writing to %d-th Mbyte \n", i>>18);
    pt[i] = i;
}
sleep(5);
free(pt);
  printf("done sweepingi and freeing!\n");
  sleep(10);
	
  printf("The end! \n");

}

