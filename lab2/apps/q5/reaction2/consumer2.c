#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "q1.h"

// Consumer 2:
// Consumes 2H2O; Produces 2 H2 and 1 O2
void main (int argc, char *argv[])
{
	// Variable declarations
	sem_t s_hho;
	sem_t s_hh;
	sem_t s_oo;
	sem_t s_proc;				
    	char usage[] = "<h_s_proc> <h_s_hho> <h_s_hh> <h_s_oo>\n";

	// Check CLA's
  	if(argc != 5) 
    {  Printf("Usage: "); Printf(argv[0]); Printf(usage); Exit();  }

  	// Convert the command-line strings into integers for use as handles
  	s_proc = dstrtol(argv[1], NULL, 10);
  	s_hho = dstrtol(argv[2], NULL, 10);
  	s_hh = dstrtol(argv[3], NULL, 10);
  	s_oo = dstrtol(argv[4], NULL, 10);

        if(sem_wait(s_hho) != SYNC_SUCCESS) 
        {  Printf("Bad semaphore s_hho (%d) in ",s_hho); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
   	Printf("REACTION2: PID %d is complete, ", Getpid());
	Printf("removed two H2O molecules, added two H2 molecules, and one O2 molecule.\n"); 
    	if(sem_signal(s_hh) != SYNC_SUCCESS)
    	{  Printf("Bad sem s_hh (%d) in ",s_hh); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
    	if(sem_signal(s_hh) != SYNC_SUCCESS)
    	{  Printf("Bad sem s_hh (%d) in ",s_hh); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
    	
	if(sem_signal(s_oo) != SYNC_SUCCESS)
    	{  Printf("Bad sem s_oo (%d) in ",s_oo); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }



    if(sem_signal(s_proc) != SYNC_SUCCESS)
    {  Printf("Bad sem s_proc (%d) in ",s_proc); Printf(argv[0]); Printf(", exiting...\n"); Exit();  }
}
