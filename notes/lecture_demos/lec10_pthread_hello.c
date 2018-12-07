/*
 * p_hello.c -- a hello program (in pthread)
 *           -- threads are independent life entities
 *           -- threads share the heap
 */
#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>

#define MAX_THREAD 1000
int global_i=99;

typedef struct {
	int id;
} parm;

void *hello(void *arg)
{
  int i,j,sum=0;
	parm *p=(parm *)arg;
	global_i +=100; // this is a race!!!!
printf("RACE >>>>> Thread %d: global_i = %d\n", p->id, global_i);
	sleep(5);
	printf("Thread %d before, address of local var i = %p (where the stack frame is), global_i=%d\n", p->id, & i,global_i);

	for (i=0;i<10000;i++)
	  for (j=0;j<100;j++)
	    sum += (i*j); 
        sum = abs(sum) %  10000000;
	printf("Hello from thread %d after, sum=%d, global_i=%d\n", p->id,sum*100+p->id,global_i);
	return (NULL);
}

int main(int argc, char* argv[]) {
	int n,i;
	pthread_t *threads;
	pthread_attr_t pthread_custom_attr;
	parm *p;

	if (argc != 2)
	{
		printf ("Usage: %s n\n  where n is no. of threads\n",argv[0]);
		exit(1);
	}

	n=atoi(argv[1]);
	if ((n < 1) || (n > MAX_THREAD))
	{
		printf ("The no of thread should between 1 and %d.\n",MAX_THREAD);
		exit(1);
	}

	threads=(pthread_t *)malloc(n*sizeof(*threads));
	pthread_attr_init(&pthread_custom_attr);

	p=(parm *)malloc(sizeof(parm)*n);
	/* Start up thread */

	for (i=0; i<n; i++)
	{
		p[i].id=i;
		pthread_create(&threads[i], &pthread_custom_attr, hello, (void *)(p+i));
	}

	/* Synchronize the completion of each thread. */

	for (i=0; i<n; i++)
	{
		pthread_join(threads[i],NULL);
	}
	free(p);
exit(0);
}
