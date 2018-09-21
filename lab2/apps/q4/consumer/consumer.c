#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "q1.h"

void main (int argc, char *argv[])
{
	// Variable declarations
  	circular_buffer * cbuf;  		// Circular buffer handle   
  	uint32 h_mem;      				// Handle to the shared memory page
  	lock_t lock; 					// Lock for exclusive code
  	lock_t l_buff;       			// Lock for buffer condvars 
  	cond_t not_full; 			    // Condition variable not_full
  	cond_t not_empty; 			    // Condition variable not_empty
	sem_t sem_proc;					// Semaphore to signal the original proc
	int i=0;						// While condition for # iters
    int now_room=0;                 // Check if buffer will have space after consuming
    char usage[] = "<h_mem_pg> <h_l_gplock> <h_s_proc> <h_l_empty> <h_l_full> <h_cv_empty> <h_cv_full>\n";

	// Check CLA's
  	if(argc != 7) 
    {  Printf("Usage: "); Printf(argv[0]); Printf(usage); Exit();  }

  	// Convert the command-line strings into integers for use as handles
  	h_mem = dstrtol(argv[1], NULL, 10);
  	lock = dstrtol(argv[2], NULL, 10);
	sem_proc = dstrtol(argv[3], NULL, 10);
	l_buff = dstrtol(argv[4], NULL, 10);
	not_empty = dstrtol(argv[5], NULL, 10);
	not_full = dstrtol(argv[6], NULL, 10); 

 	// Map shared memory page into this process's memory space
  	if((cbuf = (circular_buffer *)shmat(h_mem)) == NULL) 
    {  Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }

	// Access shared memory and place characters one by one
	while(i < 11)
	{
        // Grab lock associated with full_buff condvar
	    while(lock_acquire(l_buff) != SYNC_SUCCESS); 

        // If empty, wait 'till we get something to consume
        while(cbuf->head == cbuf->tail)
        {
            // Get in queue to be signalled that buffer is full
            cond_wait(not_empty); 
        }

        // Check if the buffer was full b4 consuming this next item...
        if(cbuf->head == (cbuf->tail+1)%BUFFERSIZE) now_room = 1;

	    // REMOVE CHAR FROM BUFFER
	    Printf("Consumer %d removed: %c\n",Getpid(),cbuf->buffer[cbuf->head]);
	    
        // UPDATE HEAD
	    cbuf->head = (cbuf->head+1)%BUFFERSIZE;
	    
        // INCREMENT COUNTER
        i++;

        // If buffer now empty, signal it!
        if(now_room == 1) 
        { 
            if(cond_signal(not_full) != SYNC_SUCCESS) 
            {  Printf("Bad cond v. empty_buff (%d) in ",not_full); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
            now_room == 0;
        }
        else cond_signal(not_full); 

        // Release the lock after all work is done (mesa style)
    	if(lock_release(l_buff) != SYNC_SUCCESS) {  Printf("ERROR, BAD LOCK RELEASE\n"); Exit();  }
    }

  	// Signal the semaphore to tell the original process that we're done
	Printf("Consumer: PID %d is complete. Killing it.\n", Getpid());
  	
    if(sem_signal(sem_proc) != SYNC_SUCCESS) 
	{  Printf("Bad semaphore s_procs_completed (%d) in ", sem_proc); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
}
