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
	sem_t sem_proc;					// Semaphore to signal the original proc
    sem_t s_fullslots;              // Semaphore to signal full slots in buffer
    sem_t s_emptyslots;             // Semaphore indicating empty slots in buffer
	int i=0;						// While condition for # iters
    char usage[] = "<handle_to_shared_memory_pg> <handle_to_lock> <handle_to_sem_proc> <h_s_emptyslots> <h_s_fullslots>\n";

	// Check CLA's
  	if(argc != 6) 
    {  Printf("Usage: "); Printf(argv[0]); Printf(usage); Exit();  }

  	// Convert the command-line strings into integers for use as handles
  	h_mem = dstrtol(argv[1], NULL, 10);
  	lock = dstrtol(argv[2], NULL, 10);
	sem_proc = dstrtol(argv[3], NULL, 10);
	s_emptyslots = dstrtol(argv[4], NULL, 10);
    s_fullslots = dstrtol(argv[5], NULL, 10);

 	// Map shared memory page into this process's memory space
  	if((cbuf = (circular_buffer *)shmat(h_mem)) == NULL) 
    {  Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }

	// Access shared memory and place characters one by one
	while(i < 11)
	{
        // Wait for something to consume
        if(sem_wait(s_fullslots) != SYNC_SUCCESS) 
        {  Printf("Bad semaphore s_fullslots (%d) in ",s_fullslots); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
		
        // We have available resources... get the lock
	    if(lock_acquire(lock) != SYNC_SUCCESS) {  Printf("ERROR, BAD LOCK AQUIRE\n"); Exit();  }

	    // REMOVE CHAR FROM BUFFER
	    Printf("Consumer %d removed: %c\n",Getpid(),cbuf->buffer[cbuf->head]);
	    
        // UPDATE HEAD
	    cbuf->head = (cbuf->head+1)%BUFFERSIZE;
	    
        // INCREMENT COUNTER
        i++;

        // Release the lock
    	if(lock_release(lock) != SYNC_SUCCESS) {  Printf("ERROR, BAD LOCK RELEASE\n"); Exit();  }

        // Signal that a empty buffer slot is now availible
        if(sem_signal(s_emptyslots) != SYNC_SUCCESS) 
        {  Printf("Bad semaphore s_emptyslots (%d) in ",s_emptyslots); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
    }

  	// Signal the semaphore to tell the original process that we're done
	Printf("Consumer: PID %d is complete. Killing it.\n", Getpid());
  	
    if(sem_signal(sem_proc) != SYNC_SUCCESS) 
	{  Printf("Bad semaphore s_procs_completed (%d) in ", sem_proc); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
}
