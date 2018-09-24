#ifndef __USERPROG__
#define __USERPROG__

typedef struct circular_buffer {
	int numprocs;
	char buffer[BUFFERSIZE];
	int head;
	int tail;
} circular_buffer;

#define PRODUCER_RUN "producer.dlx.obj"
#define CONSUMER_RUN "consumer.dlx.obj"

#define INJECTION1_RUN "producer1.dlx.obj"
#define INJECTION2_RUN "producer2.dlx.obj"
#define REACTION1_RUN "consumer1.dlx.obj"
#define REACTION2_RUN "consumer2.dlx.obj"
#define REACTION3_RUN "consumer3.dlx.obj"

#endif
