#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "q1.h"

void main (int argc, char *argv[])
{
	// Variable declarations
  	circular_buffer * cbuf;  		// Circular buffer handle
  	uint32 h_mem;      				// Handle to the shared memory page
  	lock_t lock; 					// Lock for critical code
  	lock_t l_buff;       			// Lock for buffer condvars 
  	cond_t not_full; 			    // Condition variable not_full
  	cond_t not_empty; 			    // Condition variable not_empty
	sem_t sem_proc;					// Semaphore to signal the original proc
	int i=0;						// While condition for # iters
    int now_something=0;            // Check if buffer will b full after producing
	char hello[11] = "Hello World";	// Value placed in buffer
    char usage[] = "<h_shared_memory_pg> <h_lock> <h_proc_semaphore> <h_l_buff> <h_c_notEmpty> <h_c_notFull>\n";

	// Check CLA's
  	if (argc != 7) 
    {  Printf("Usage: "); Printf(argv[0]); Printf(usage); Exit();  }

  	// Convert the command-line strings into integers for use as handles
  	h_mem = dstrtol(argv[1], NULL, 10);
  	lock = dstrtol(argv[2], NULL, 10);
	sem_proc = dstrtol(argv[3], NULL, 10);
	l_buff = dstrtol(argv[4], NULL, 10);
	not_empty = dstrtol(argv[5], NULL, 10);
	not_full = dstrtol(argv[6], NULL, 10); 

 	// Map shared memory page into this process's memory space
  	if ((cbuf = (circular_buffer *)shmat(h_mem)) == NULL) 
    {  Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }

	// Access shared memory and place characters one by one
	while(i < 11)
	{
        // Grab lock associated with buffer condvar's
	    while(lock_acquire(l_buff) != SYNC_SUCCESS);

        // If full, wait 'till not_full gets signaled
        while(cbuf->head == (cbuf->tail+1)%BUFFERSIZE)
        {
            // Get in queue to be signalled when buffer has space
            cond_wait(not_full);
        }

        // Check if the buffer was previously empty b4 producing this next item...
        if(cbuf->head == (cbuf->tail+1)%BUFFERSIZE) now_something = 1;
		
        // ADD CHAR TO BUFFER
		cbuf->buffer[cbuf->tail] = hello[i];
		Printf("Producer %d inserted: %c\n",Getpid(), cbuf->buffer[cbuf->tail]);
		
        // UPDATE TAIL
		cbuf->tail = (cbuf->tail+1)%BUFFERSIZE;
        
        // UPDATE INCREMENTER
		i++;

        // If buffer now has something (and previously was empty), signal it!
        if(now_something == 1) 
        { 
            if(cond_signal(not_empty) != SYNC_SUCCESS) 
            {  Printf("Bad cond v. empty_buff (%d) in ",not_empty); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
            now_something = 0;	
        }
        else cond_signal(not_empty); 

        // Release the lock after all work is done (mesa style)
    	if(lock_release(l_buff) != SYNC_SUCCESS) {  Printf("ERROR, BAD LOCK RELEASE\n"); Exit();  }
	}
  	Printf("Producer: PID %d is complete. Killing it. Moving on to consumer...\n", Getpid()); 
}
