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
	sem_t sem_proc;					// Semaphore to signal the original process
    sem_t s_emptyslots;             // Semaphore to indicate # empty slots in buffer
    sem_t s_fullslots;              // Semaphore to indicate # full slots in buffer
	int i=0;						// While condition for # iters
	char hello[11] = "Hello World";	// Value placed in buffer
    char usage[] = "<h_shared_memory_pg> <h_lock> <h_proc_semaphore> <h_s_emptyslots> <h_s_fullslots>\n";

	// Check CLA's
  	if (argc != 6) 
    {  Printf("Usage: "); Printf(argv[0]); Printf(usage); Exit();  }

  	// Convert the command-line strings into integers for use as handles
  	h_mem = dstrtol(argv[1], NULL, 10);
  	lock = dstrtol(argv[2], NULL, 10);
	sem_proc = dstrtol(argv[3], NULL, 10);
    s_emptyslots = dstrtol(argv[4], NULL, 10);
    s_fullslots = dstrtol(argv[5],NULL,10);

 	// Map shared memory page into this process's memory space
  	if ((cbuf = (circular_buffer *)shmat(h_mem)) == NULL) 
    {  Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }

	// Access shared memory and place characters one by one
	while(i < 11)
	{
        // Wait for space to produce new character
        if(sem_wait(s_emptyslots) != SYNC_SUCCESS)
        {  Printf("Bad semaphore"); Printf(" exitting...\n"); Exit();  }
	
        // We have some space... go get the lock
		if(lock_acquire(lock) != SYNC_SUCCESS)
		{  Printf("ERROR AQUIRING LOCK \n"); Printf("exitting...\n"); Exit();  }

		// ADD CHAR TO BUFFER
		cbuf->buffer[cbuf->tail] = hello[i];
		Printf("Producer %d inserted: %c\n",Getpid(), cbuf->buffer[cbuf->tail]);
		
        // UPDATE TAIL
		cbuf->tail = (cbuf->tail+1)%BUFFERSIZE;
        
        // UPDATE INCREMENTER
		i++;
		
		// Release the lock
		if (lock_release(lock) != SYNC_SUCCESS)
		{  Printf("ERROR RELEASING LOCK %d", Getpid()); Printf(" exitting...\n"); Exit();  }

        // Signal that we have one more full slot in the buffer now...
        if(sem_signal(s_fullslots) != SYNC_SUCCESS)
        {  Printf("Bad sem s_fullslots (%d) in ",s_fullslots); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
	}
 
  	Printf("Producer: PID %d is complete. Killing it. Moving on to consumer...\n", Getpid());
}
