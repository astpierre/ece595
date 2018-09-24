#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "q1.h"

// Producer for NN

void main (int argc, char *argv[])
{
	// Variable declarations
	sem_t s_nn;					// Semaphore to signal the original proc
	sem_t s_proc;					// Semaphore to signal the original proc
    char usage[] = "<h_proc_semaphore> <h_nn_semaphore>\n";

	// Check CLA's
  	if (argc != 3) 
    {  Printf("Usage: "); Printf(argv[0]); Printf(usage); Exit();  }

 	// Convert the command-line strings into integers for use as handles
	s_proc = dstrtol(argv[1], NULL, 10);
  	s_nn = dstrtol(argv[2], NULL, 10); 

    if(sem_signal(s_nn) != SYNC_SUCCESS)
    {  Printf("Bad sem s_nn (%d) in ",s_nn); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }

    Printf("INJECTOR1: PID %d is complete, added one N2 molecule.\n", Getpid()); 

    if(sem_signal(s_proc) != SYNC_SUCCESS)
    {  Printf("Bad sem s_proc (%d) in ",s_proc); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
}
