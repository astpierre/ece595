//
//	queue.c
//
//	Variables for queues, and a routine to initialize the pool of links.
//
//

#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"

Queue		freeLinks;  // Stores all the free links in the system
static Link	linkpool[QUEUE_MAX_LINKS]; // Memory space for each link, since we can't use malloc()

//-------------------------------------------------------------------------

///////////////////////////////////////////////////////
//
// Queue and Link Initialization
//
///////////////////////////////////////////////////////

//-------------------------------------------------------------------------

///////////////////////////////////////////////////////
// Initializes the Queue module
///////////////////////////////////////////////////////
void testfuncvoid() {
  dbprintf('q', "testfunvoid\n");
}
int testfuncint() {
  dbprintf('q', "testfuncint\n");
  return 0;
}
int retzero();

int AQueueModuleInit() {
  int i;
  if (AQueueInit(&freeLinks) != QUEUE_SUCCESS) {
    printf("FATAL ERROR: could not initialize freeLinks queue in AQueueModuleInit!\n");
    GracefulExit();
  }
  dbprintf ('q', "Initializing %d links.\n", QUEUE_MAX_LINKS);
  for (i = 0; i < QUEUE_MAX_LINKS; i++) {
    // Initialize link structure
    linkpool[i].next = NULL;
    linkpool[i].prev = NULL;
    linkpool[i].object = NULL;
    // Add link to freeLinks queue
    if (AQueueInsertLast(&freeLinks, &(linkpool[i])) != QUEUE_SUCCESS) {
      printf("FATAL ERROR: could not insert link into freeLinks in AQueueModuleInit!\n"); // Adds structure to global queue of free links
      GracefulExit();
    }
  }
  return QUEUE_SUCCESS;
}

///////////////////////////////////////////////////////
// Initializes a given Queue to zero items
///////////////////////////////////////////////////////
int AQueueInit (Queue *q) {
  if (!q) return QUEUE_FAIL;
  q->first = NULL;
  q->last = NULL;
  q->nitems = 0;
  return QUEUE_SUCCESS;
}


//-------------------------------------------------------------------------

///////////////////////////////////////////////////////
//
// Queue and Link Allocation, Removal, and Freeing
//
///////////////////////////////////////////////////////

//-------------------------------------------------------------------------

///////////////////////////////////////////////////////
// Gets an empty link structure from the global queue
// of empty links.
///////////////////////////////////////////////////////
Link *AQueueAllocLink (void *obj_to_store) {
  extern Queue	freeLinks;
  Link	*l=NULL;

  dbprintf('q', "AQueueAllocLink: allocating link\n");
  if (AQueueEmpty(&freeLinks)) {
    dbprintf('q', "AQueueAllocLink: no free links!\n");
    return NULL;
  }
  l = AQueueFirst(&freeLinks);
  if (!l) {
    dbprintf('q', "AQueueAllocLink: first link in freeLinks is NULL!\n");
    return NULL;
  }

  // Can't use QueueRemove because it puts the link back onto
  // the list of free links.  Therefore, we must remove it
  // manually.
  
  // First fix freeLinks structure
  if (freeLinks.first == l) freeLinks.first = l->next;
  if (freeLinks.last == l) freeLinks.last = l->prev;
  freeLinks.nitems--;

  // Next, fix list around l
  if (l->prev) l->prev->next = l->next;
  if (l->next) l->next->prev = l->prev;

  // Finally, reset l's internal pointers to NULL for safety
  l->next = NULL;
  l->prev = NULL;
  l->queue = NULL;
  l->object = obj_to_store;

  return l;
}

/////////////////////////////////////////////////////////////////
// Removes link "l" from the queue that it belongs to, and
// adds it back to the global queue of free links.
/////////////////////////////////////////////////////////////////
int AQueueRemove (Link **pl) {
  extern Queue freeLinks;
  Link *l = NULL;

  dbprintf('q', "AQueueRemove: removing link\n");

  if (!pl) return QUEUE_FAIL;
  if (!(*pl)) return QUEUE_FAIL;
  l = *pl; // Just for convenience so I don't have to keep typing *pl
  if (!l->queue) return QUEUE_FAIL;

  // First, fix the queue's first and last pointers
  if (AQueueFirst(l->queue) == l) l->queue->first = l->next; // l was first item on queue
  if (AQueueLast(l->queue) == l)  l->queue->last = l->prev;  // l was last item on queue

  // Next, reconnect the list around l
  if (l->prev) l->prev->next = l->next;
  if (l->next) l->next->prev = l->prev;

  // Update the number of items in the queue
  l->queue->nitems--;

  // Clear the link, and add it back to the link back to the list of free links
  AQueueInsertLast(&freeLinks, l);

  *pl = NULL;
  return QUEUE_SUCCESS;
}

//-------------------------------------------------------------------------

///////////////////////////////////////////////////////
//
// Link Insertion
//
///////////////////////////////////////////////////////

//-------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////
// Inserts link "l" after link "after" in queue "q"
/////////////////////////////////////////////////////////////////
int AQueueInsertAfter (Queue *q, Link *after, Link *l) {
  if (!l) return QUEUE_FAIL;
  if (!q) return QUEUE_FAIL;
 
  if (after) { //  If after is not NULL, then after must be on the queue q
    if (after->queue != q) return QUEUE_FAIL;
  } else { // Otherwise after is NULL, so the list has to be empty
    if (q->first != NULL) return QUEUE_FAIL;
    if (q->last != NULL) return QUEUE_FAIL;
    if (!AQueueEmpty(q)) return QUEUE_FAIL;
  }
  
  // Save the queue inside the link for simplicity
  l->queue = q;

  // Set next and previous on the link itself
  l->prev = after;
  if (after) l->next = after->next;
  else       l->next = NULL;

  // Fix the list around l
  if (after) after->next = l;
  if (l->next) l->next->prev = l;

  // Update the queue accordingly for this new node
  if (!after) {
    q->first = l;
    q->last = l;
    q->nitems = 1;
  } else {
    // Note: don't need to worry about if "after" was first item, because 
    // we're inserting after it, not before it
    if (q->last == after) q->last = l;
    q->nitems++;
  }
  return QUEUE_SUCCESS;
}

//////////////////////////////////////////////////////////////////
// Inserts link "l" before link "before" in queue "q"
//////////////////////////////////////////////////////////////////
int AQueueInsertBefore(Queue *q, Link *before, Link *l) {
  if (!l) return QUEUE_FAIL;
  if (!q) return QUEUE_FAIL;
  
  if (before) { //  If before is not NULL, then before must be on the queue q
    if (before->queue != q) return QUEUE_FAIL;
  } else { // Otherwise before is NULL, so the list has to be empty
    if (AQueueFirst(q) != NULL) return QUEUE_FAIL;
    if (AQueueLast(q) != NULL) return QUEUE_FAIL;
    if (!AQueueEmpty(q)) return QUEUE_FAIL;
  }
  
  // Save the queue inside the link for simplicity
  l->queue = q;

  // Set next and previous on the link itself
  l->next = before;
  if (before) l->prev = before->prev;
  else        l->prev = NULL;

  // Fix the list around l
  if (before) before->prev = l;
  if (l->prev) l->prev->next = l;

  // Update the queue accordingly for this new node
  if (!before) {
    q->first = l;
    q->last = l;
    q->nitems = 1;
  } else {
    // Note: don't need to worry about if "before" was last item, because 
    // we're inserting before it, not after it
    if (q->first == before) q->first = l;
    q->nitems++;
  }
  return QUEUE_SUCCESS;
}

////////////////////////////////////////////////////////////////////
// Moves link "l" to the position after link "after" in queue "q"
////////////////////////////////////////////////////////////////////
int AQueueMoveAfter(Queue *q, Link *after, Link *l) {
  if (!q) return QUEUE_FAIL;
  if (!l) return QUEUE_FAIL;
  if (l->queue != q) return QUEUE_FAIL; // l does not belong to q

  // If after is NULL, then l is the last item on the queue,
  // so we don't have to do anything
  if (!after) {
    if (AQueueLength(q) != 1) return QUEUE_FAIL;
    if (AQueueFirst(q) != AQueueLast(q)) return QUEUE_FAIL;
    return QUEUE_SUCCESS;
  }
  if (after->queue != q) return QUEUE_FAIL;
  if (after == l) { // don't have to do anything
    return QUEUE_SUCCESS;
  }

  // First fix Queue first/last pointers
  if (q->last == l) return QUEUE_FAIL; // this would be strange if it happened
  if (q->first == l) q->first = l->next;
  if (q->last == after) q->last = l;

  // Now remove l from it's original position in the list
  if (l->next) l->next->prev = l->prev;
  if (l->prev) l->prev->next = l->next;

  // Now update l's prev and next pointers to put l into the list after the link "after"
  l->prev = after;
  l->next = after->next;

  // Now fix the list around l
  after->next = l;
  if (l->next) l->next->prev = l;

  return QUEUE_SUCCESS;
}

////////////////////////////////////////////////////////////////////
// Inserts link "l" as the first link in the Queue "q"
////////////////////////////////////////////////////////////////////
int AQueueInsertFirst (Queue *q, Link *l) { 
  if (!l) return QUEUE_FAIL;
  if (!q) return QUEUE_FAIL;
  return AQueueInsertBefore(q, AQueueFirst(q), l); 
}

////////////////////////////////////////////////////////////////////
// Inserts link "l" as the last link in the Queue "q"
////////////////////////////////////////////////////////////////////
int AQueueInsertLast(Queue *q, Link *l) { 
  if (!l) return QUEUE_FAIL;
  if (!q) return QUEUE_FAIL;
  return AQueueInsertAfter(q, AQueueLast(q), l); 
}


//-------------------------------------------------------------------------

///////////////////////////////////////////////////////
//
// Access functions
//
///////////////////////////////////////////////////////

//-------------------------------------------------------------------------


// Returns the next pointer of link "l"
Link *AQueueNext(Link *l ) { return l->next; }
// Returns the previous pointer of link "l"
Link *AQueuePrev(Link *l ) { return l->prev; }
// Returns the first link in the queue
Link *AQueueFirst(Queue *q) { return q->first; }
// Returns the last link in the queue
Link *AQueueLast(Queue *q) { return q->last; }
// Returns the object pointer in a link
void *AQueueObject(Link *l) { return l->object; }
// Returns the number of items in the queue
int AQueueLength(Queue *q) { return q->nitems; }
// Returns true is the queue is empty, false otherwise
int AQueueEmpty (Queue *q) { return (AQueueLength (q) == 0); }


