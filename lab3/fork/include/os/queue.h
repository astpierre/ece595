//
//	queue.h
//
//	Definitions for routines used to manage queues.  The queues are
//	doubly-linked lists so it's easier to add and remove from them.
//	Also, the links themselves have pointers to objects so that objects
//	may be on more than one queue.
//

#ifndef	_queue_h_
#define	_queue_h_

#ifndef	NULL
#define	NULL	((void *)0)
#endif

#define	QUEUE_MAX_LINKS		400

#define QUEUE_FAIL 0
#define QUEUE_SUCCESS 1

// Doubly-linked list nodes for storing objects
typedef struct Link {
  struct Link *next;
  struct Link *prev;
  struct Queue *queue;
  void	*object;
} Link;

// Used to store a list of links
typedef struct Queue {
  struct Link *first;
  struct Link *last;
  int nitems;
} Queue;

// Returns the next pointer of a link
Link *  AQueueNext(Link *l );

// Returns the previous pointer of a link
Link *  AQueuePrev(Link *l );

// Returns the first element in the queue
Link * AQueueFirst(Queue *q);

// Returns the last element in the queue
Link *  AQueueLast(Queue *q);

// Returns the object pointer inside a link
void *AQueueObject(Link *l);

// Returns the number of elements in a queue
int AQueueLength(Queue *q);

// Returns true is queue is empty, false otherwise
int AQueueEmpty (Queue *q);

// Inserts the link "l" after the link "after" in the queue "q"
int AQueueInsertAfter (Queue *q, Link *after, Link *l);

// Inserts the link "l" as the first item in the queue
int AQueueInsertFirst (Queue *q, Link *l);

// Inserts the link "l" as the last link in the queue
int AQueueInsertLast (Queue *q, Link *l);

// Moves link "l" to the position after link "after" in queue "q"
int AQueueMoveAfter(Queue *q, Link *after, Link *l);

// Removes link "l" from the queue that it belongs to
// and sets *l = NULL
int AQueueRemove (Link **l);

// Initializes the Queue module
int AQueueModuleInit ();

// Gets a free link from the queue of free links
Link *AQueueAllocLink (void *pointer_to_store_in_queue);

// Initializes a queue to have zero items
int AQueueInit (Queue *q);

#endif	// _queue_h_
