#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "q1.h"

void main (int argc, char *argv[])
{
	// Variable declarations
	int numprocs = 0;               // Used to store number of processes to create
  	int i;                          // Loop index variable
  	circular_buffer * cbuf;         // Used to get address of shared memory page
  	uint32 h_mem;                   // Used to hold handle to shared memory page
 	sem_t s_procs_completed;        // Semaphore used wait until procs complete
    sem_t s_fullslots;              // Semaphore indicating # of full slots in buffer
    sem_t s_emptyslots;             // Semaphore indicating # of empty slots in buffer
	lock_t l_proc;					// Lock used to indicated critcal sections
    char h_mem_str[10];             // Used as CLA to pass mem_handle to procs
  	char s_procs_completed_str[10]; // Used as CLA to pass page_mapped hndl to procs	
	char l_proc_str[10];			// Used as CLA to pass lock handle to procs
    char s_fullslots_str[10];       // Used as CLA to pass fullslots sema handle
    char s_emptyslots_str[10];      // Used as CLA to pass emptyslots sema handle

	// Check CLA's
  	if (argc != 2) 
	{  Printf("Usage: "); Printf(argv[0]); Printf(" <number of processes to create>\n"); Exit();  }

  	// Convert string from ascii command line argument to integer number
  	numprocs = dstrtol(argv[1], NULL, 10);
  	Printf("Creating %d processes...\n", numprocs);

  	// Allocate space for a shared memory page, which is exactly 64KB 
  	if ((h_mem = shmget()) == 0) 
	{  Printf("ERROR: could not allocate shared memory page in "); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
  	
	// Map shared memory page into this process's memory space
  	if ((cbuf = (circular_buffer *)shmat(h_mem)) == NULL) 
	{  Printf("Could not map the shared page to v address in "); Printf(argv[0]); Printf(", exiting..\n"); Exit();  }

  	// Initialize the circular buffer
	cbuf->head = 0;
	cbuf->tail = 0;

  	// Create semaphore to not exit this process until all other processes
  	// have signalled that they are complete. Initialize to (-1)*(numprocs)  
	if ((s_procs_completed = sem_create(-(numprocs-1))) == SYNC_FAIL) 
	{  Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n"); Exit();  }

    // Create semaphore to indicate the number of full slots in the buffer
    // Initialize to 0 b/c buffer is empty (nothing full)
    if ((s_fullslots = sem_create(0)) == SYNC_FAIL)
    {  Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n"); Exit();  }

    // Create semaphore to indicate the number of empty slots in the buffer
    // Initialize to BUFFERSIZE b/c buffer is empty (totally empty)
    if ((s_emptyslots = sem_create(BUFFERSIZE)) == SYNC_FAIL)
    {  Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n"); Exit();  }

	// Create a lock, check fail
	if((l_proc = lock_create()) == SYNC_FAIL)
	{  Printf("ERROR: PROCESS COULD NOT INITIALIZE LOCK\n"); Exit();  }

  	// Setup the command-line arguments for the new processes
  	ditoa(h_mem, h_mem_str);
	ditoa(l_proc, l_proc_str);
  	ditoa(s_procs_completed, s_procs_completed_str);
    ditoa(s_fullslots, s_fullslots_str);
    ditoa(s_emptyslots, s_emptyslots_str);

	// Now we can create the processes 
  	for (i=0; i<numprocs; i++) 
	{
		// Pass each process the handle to memory, lock handle, semaphore handles
   		process_create(PRODUCER_RUN, h_mem_str, l_proc_str, s_procs_completed_str, s_emptyslots_str, s_fullslots_str, NULL);
		process_create(CONSUMER_RUN, h_mem_str, l_proc_str, s_procs_completed_str, s_emptyslots_str, s_fullslots_str, NULL);
	}

  	// And finally, wait until all spawned processes have finished.
  	if (sem_wait(s_procs_completed) != SYNC_SUCCESS) 
	{  Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n"); Exit();  }

	// Done here, exiting
  	Printf("All producers/consumers have completed, exiting main process.\n");
}
