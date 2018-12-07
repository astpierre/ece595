#include <stdio.h>
#include <sys/time.h>

#define SIZE 1024*1024*768
double sqrt();
struct	timeval	start, finish;

print_time()
{
  printf(" time: %.2f seconds\n", (((finish.tv_sec * 1000000.0) + finish.tv_usec) -
	  ((start.tv_sec * 1000000.0) + start.tv_usec))/1000000.0);
}

main()
{
  int *ptr, i,j,k,kk;

  ptr = (int *) malloc(SIZE);
  printf("SIZE = %d ptr=%p\n", SIZE, ptr);

  // first time touching every page

  gettimeofday(&start, NULL);
  for (i=1; i< (SIZE>>2); i+=1024) 
   *(ptr + i) = (int) sqrt((double) (*(ptr + i - 1) * 3.0));
  gettimeofday(&finish, NULL);
  print_time();

  // second time touching every page

  gettimeofday(&start, NULL);
  for (i=1; i< (SIZE>>2); i+=1024) 
   *(ptr + i) = (int) sqrt((double) (*(ptr + i - 1) * 3.0));
  gettimeofday(&finish, NULL);
  print_time();
  printf("-----------------------------------------------\n");

  printf("touch every 4 bytes, total 100k times\n");

  kk = SIZE >> 12;  
  printf(" kk = %d \n",kk);
  k = kk;
  gettimeofday(&start, NULL); 
  for (j=0; j<100; j++)
    for (i=0; i<k; i++) 
      *(ptr + i) = (int) ((double) (*(ptr + i) * 3.0));
  gettimeofday(&finish, NULL);
  print_time();

  printf("touch every other 32 bytes -- cacheline -- total 100k times\n");

  k = kk; 
  gettimeofday(&start, NULL); 
  for (j=0; j<100; j++)
    for (i=0; i < 8*k; i+=8) 
      *(ptr + i) = (int) ((double) (*(ptr + i) * 3.0));
  gettimeofday(&finish, NULL);
  print_time();

  printf("touch every other 4096 bytes -- a page -- total 100k times\n");

  k = kk;
  gettimeofday(&start, NULL); 
  for (j=0; j<100; j++)
    for (i=0; i<1024*k; i+=1024) 
      *(ptr + i) = (int) ((double) (*(ptr + i) * 3.0));
  gettimeofday(&finish, NULL);
  print_time();
}
