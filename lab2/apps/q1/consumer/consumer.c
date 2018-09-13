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
	int i=0;						// Incrementer
	int next;						// Circular buffer accessory

	// Check CLA's
  	if (argc != 4) 
    { 
        Printf("Usage: "); 
        Printf(argv[0]); 
        Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_lock> <handle_to_page_mapped_semaphore>\n"); 
        Exit();
  	}

  	// Convert the command-line strings into integers for use as handles
  	h_mem = dstrtol(argv[1], NULL, 10);
  	lock = dstrtol(argv[2], NULL, 10);
	sem_proc = dstrtol(argv[3], NULL, 10);

 	// Map shared memory page into this process's memory space
  	if ((cbuf = (circular_buffer *)shmat(h_mem)) == NULL) {
    	Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n"); Exit();
 	}


	// Access shared memory and place characters one by one
	while(i < 11)
	{
		// Get the lock
		if(lock_acquire(lock))
		{
			// If !EMPTY
			if(cbuf->tail != cbuf->head)
			{
				// REMOVE CHAR FROM BUFFER
				Printf("Consumer %d removed: %c\n",Getpid(),cbuf->buffer[cbuf->head]);
				// UPDATE HEAD
				cbuf->head = (cbuf->head+1)%BUFFERSIZE;
				i++;
			}
			// Release the lock
			if (lock_release(lock) != SYNC_SUCCESS)
			{
				Printf("ERROR");
			}
		}
	}

  	// Signal the semaphore to tell the original process that we're done
	Printf("Consumer: PID %d is complete. Killing it.\n", Getpid());
  	if(sem_signal(sem_proc) != SYNC_SUCCESS) 
	{
		Printf("Bad semaphore s_procs_completed (%d) in ", sem_proc); 
		Printf(argv[0]); 
		Printf(", exiting...\n");
	    Exit();
  	}
}
