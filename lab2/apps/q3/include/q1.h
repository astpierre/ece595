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

#endif
