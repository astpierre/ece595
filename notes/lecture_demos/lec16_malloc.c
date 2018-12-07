#include <stdio.h>
#include <sys/time.h>

int main()
{
  int i, *p1, *p2, *p3, *p4;

  // let us directly write to the top of the heap without malloc!
  p1 = (int *)0x804a040;
  printf(" p1 = %p in page %p\n", p1, ((int) p1>>12)<<12);
  *p1 = 1;
  printf("*p1 = %d\n", *p1);

  p1 = (int *)0x804b000;
  printf(" p1 = %p in page %p\n", p1, ((int) p1>>12)<<12);
  *p1 = 2;
  printf("*p1 = %d\n", *p1);

  /*
  p1 = (int *)0x804c000;
  printf(" p1 = %p in page %p\n", p1, ((int) p1>>12)<<12);
  *p1 = 3;
  printf("*p1 = %d\n", *p1);
  */

  // ----------------------------------------------------------
  // this shows the current top of the heap is not page aligned
  if ((p1 = (int *) malloc(24)) != NULL)
    printf(" p1 = %p in page %p\n", p1, ((int) p1>>12)<<12);
  free(p1);

  // after mallocing something big --> force page alignment
  if ((p1 = (int *) malloc(4096)) != NULL)
    printf(" p1 = %p in page %p\n", p1, ((int) p1>>12)<<12);
  
  // now, what address will this malloc() return?
  if ((p1 = (int *) malloc(24)) != NULL)
    printf(" p1 = %p in page %p\n", p1, ((int) p1>>12)<<12);

  // what does free() really do?
  free(p1);
  printf("*p1 = %d\n", *p1);
  
  exit(0);
}


