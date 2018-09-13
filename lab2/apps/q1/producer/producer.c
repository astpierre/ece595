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
	sem_t sem_proc;					// Semaphore to signal the original process
	int i=0;						// Incrementer
	char hello[12] = "Hello World";	// Value placed in buffer	
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
			// If !FULL
			if(cbuf->head != (cbuf->tail+1)%BUFFERSIZE)
			{
				// ADD CHAR TO BUFFER
				cbuf->buffer[cbuf->tail] = hello[i];
				Printf("Producer %d inserted: %c\n",Getpid(), cbuf->buffer[cbuf->tail]);
				// UPDATE TAIL
				cbuf->tail = (cbuf->tail+1)%BUFFERSIZE;
				i++;
			}
			// Release the lock
			if (lock_release(lock) != SYNC_SUCCESS)
			{
				Printf("ERROR RELEASING LOCK %d", Getpid());
			}
		}
	}

  	// Signal the semaphore to tell the original process that we're done
  	Printf("Producer: PID %d is complete. Killing it. Moving on to consumer...\n", Getpid());
  	/*if(sem_signal(sem_proc) != SYNC_SUCCESS) 
	{
		Printf("Bad semaphore s_procs_completed (%d) in ", sem_proc); 
		Printf(argv[0]); 
		Printf(", exiting...\n");
	    Exit();
  	}*/
}
