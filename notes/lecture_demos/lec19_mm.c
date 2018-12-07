// what are the working set sizes of these?
// assuming cache is as big as N, and 3*b^2 can fit in cache, 

#include <stdio.h>
#include <sys/time.h>

#define SIZE 512
int A[SIZE][SIZE], B[SIZE][SIZE], C1[SIZE][SIZE], C2[SIZE][SIZE];
struct	timeval	start, stop;

print_time()
{ printf(" time: %.2f seconds\n",
	 (((stop.tv_sec * 1000000.0) + stop.tv_usec) -
	  ((start.tv_sec * 1000000.0) + start.tv_usec))/1000000.0);
}

matrix_multiply_1()
{
  int i,j,k;
  for (i=0; i<SIZE; i++) {
    for (j=0; j<SIZE; j++) {
      for (k=0; k<SIZE; k++) {
      C1[i][j] += A[i][k]*B[k][j];
}}}}

matrix_multiply_2(int block_size)
{
  int i,j,k, ii,jj,kk;

  for (i=0; i<SIZE; i+=block_size) {
    for (j=0; j<SIZE; j+=block_size) {
      for (k=0; k<SIZE; k+=block_size) {
	for (ii=0; ii<block_size; ii++) {
	  for (jj=0; jj<block_size; jj++) {
	    for (kk=0; kk<block_size; kk++) {
	      C2[i+ii][j+jj] += A[i+ii][k+kk]*B[k+kk][j+jj];
}}}}}}}

main()
{
  int i,j,k, p;

  for (i=0; i<SIZE; i++) {
    for (j=0; j<SIZE; j++) {
      A[i][j] = i*2 + j*3;         B[i][j] = i*3 + j*4;
      C1[i][j] = i*4 + j*5;       C2[i][j] = i*4 + j*5;
  }}

  gettimeofday(&start, NULL);
  matrix_multiply_1();
  gettimeofday(&stop, NULL);  print_time();

  gettimeofday(&start, NULL);
  printf("block_size = %d \n", 2);
  matrix_multiply_2(2);
  gettimeofday(&stop, NULL);  print_time();

  for (i=0; i<SIZE; i++) {
    for (j=0; j<SIZE; j++) {
      if (C1[i][j] != C2[i][j])	{ printf("Bug !!! %d %d \n",C1[i][j],C2[i][j]); exit(1); }
    }
  }

  for (p=4; p<=256; p*=2) {
    gettimeofday(&start, NULL);
    printf("block_size = %d \n", p);
    matrix_multiply_2(p);
    gettimeofday(&stop, NULL);  print_time();
  }
}
