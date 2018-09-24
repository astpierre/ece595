#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "q1.h"

// Consumer 1:
// Consumes NN; Produces N and N
void main (int argc, char *argv[])
{
	// Variable declarations
	sem_t s_nn;
	sem_t s_n;
	sem_t s_proc;				
	char usage[] = "<h_s_proc> <h_s_nn> <h_s_n>\n";

	// Check CLA's
  	if(argc != 4) 
    	{  Printf("Usage: "); Printf(argv[0]); Printf(usage); Exit();  }

  	// Convert the command-line strings into integers for use as handles
  	s_proc = dstrtol(argv[1], NULL, 10);
  	s_nn = dstrtol(argv[2], NULL, 10);
  	s_n = dstrtol(argv[3], NULL, 10);

        if(sem_wait(s_nn) != SYNC_SUCCESS) 
        {  Printf("Bad semaphore s_nn (%d) in ",s_nn); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
    Printf("REACTION1: PID %d is complete, ", Getpid());
	Printf(" removed N2, added two N atoms.\n"); 
    if(sem_signal(s_n) != SYNC_SUCCESS)
    {  Printf("Bad sem s_n (%d) in ",s_n); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
    if(sem_signal(s_n) != SYNC_SUCCESS)
    {  Printf("Bad sem s_n (%d) in ",s_n); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }


    if(sem_signal(s_proc) != SYNC_SUCCESS)
    {  Printf("Bad sem s_proc (%d) in ",s_proc); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
}
