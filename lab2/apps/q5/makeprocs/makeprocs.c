#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "q1.h"

void main (int argc, char *argv[])
{
	// Variable declarations
	int num_nn = 0;                 // Used to store number of processes to create
	int num_hho = 0;		// Used to store number of hho elements
	int r3_count=0;
	int expected_procs=0; 		// Expected total # processes

	int tmp=0;			// Tmp variable
  	int i=0;                        // Loop index variable
 	sem_t s_procs_completed;        // Semaphore used wait until procs complete
  	char s_procs_completed_str[10]; // Used as CLA to pass page_mapped hndl to procs	

	sem_t s_nn;		 // N2 (input)
	sem_t s_hho;		 // H20 (input)
	sem_t s_n;		 // N
	sem_t s_hh;		 // H2 (final product)
	sem_t s_oo;		 // O2
	sem_t s_noo;		 // NO2 (final product)
	char s_nn_str[10];       // Used as CLA to pass nn sema handle
	char s_hho_str[10];      // Used as CLA to pass hho sema handle
	char s_n_str[10];        // Used as CLA to pass n sema handle
	char s_hh_str[10];       // Used as CLA to pass hh sema handle
	char s_oo_str[10];       // Used as CLA to pass oo sema handle
	char s_noo_str[10];      // Used as CLA to pass noo sema handle

	char final_hho[10];
	char final_noo[10];


	// Check CLA's
  	if (argc != 3) 
	{  Printf("Usage: "); Printf(argv[0]); Printf(" <number of n2 molecules> <number of h2o molecules>\n"); Exit();  }

  	// Convert string from ascii command line argument to integer number
  	num_nn = dstrtol(argv[1], NULL, 10);
  	Printf("Injecting %d N2...\n", num_nn);
  	num_hho = dstrtol(argv[2], NULL, 10);
  	Printf("Injecting %d H2O...\n", num_hho);

	// Use floor division to calculate the max # output 
	if ((num_nn * 2) > (num_hho / 2)) r3_count = (num_hho/2);
	else r3_count = (num_nn * 2);
	expected_procs += r3_count;
	expected_procs += num_nn * 2;
	expected_procs += num_hho / 2;
	expected_procs += num_hho / 2;

  	// Create semaphore to indicate the number of n molecules
  	// have signalled that they are complete. Initialize to (-2)*(numnn)
	if ((s_n = sem_create(0)) == SYNC_FAIL)
	{  Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n"); Exit();  }

  	// Create semaphore to indicate the number of hh molecules
  	// have signalled that they are complete. Initialize to (-1)*(numhho)  
	if ((s_hh = sem_create(0)) == SYNC_FAIL)
	{  Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n"); Exit();  }

  	// Create semaphore to indicate the number of oo molecules
  	// have signalled that they are complete. Initialize to (-1)*(numhho/2)  
	if ((s_oo = sem_create(0)) == SYNC_FAIL) 
	{  Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n"); Exit();  }

  	// Create semaphore to indicate the number of noo molecules
  	// have signalled that they are complete. Initialize to (-1)*(min{n,oo})  
	if ((s_noo = sem_create(0)) == SYNC_FAIL) 
	{  Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n"); Exit();  }

	if ((s_nn = sem_create(0)) == SYNC_FAIL) 
	{  Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n"); Exit();  }

	if ((s_hho = sem_create(0)) == SYNC_FAIL) 
	{  Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n"); Exit();  }

  	// Create semaphore to not exit this process until all other processes
  	// have signalled that they are complete. Initialize to (-1)*(numnn + numhho)  
	if ((s_procs_completed = sem_create(-(expected_procs-1))) == SYNC_FAIL) 
	{  Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n"); Exit();  }

  	// Setup the command-line arguments for the new processes
  	ditoa(s_procs_completed, s_procs_completed_str);
	ditoa(s_n, s_n_str);
	ditoa(s_hh, s_hh_str);
	ditoa(s_oo, s_oo_str);
	ditoa(s_noo, s_noo_str);
	ditoa(s_nn, s_nn_str);
	ditoa(s_hho, s_hho_str);

	// Now we can create the processes
	// Modify parameters to pass in
	for (i=0; i<r3_count; i++)
	{  process_create(REACTION3_RUN, s_procs_completed_str, s_n_str, s_oo_str, s_noo_str, NULL);  }
	for (i=0; i<(num_hho/2); i++)
	{  process_create(REACTION2_RUN, s_procs_completed_str, s_hho_str, s_hh_str, s_oo_str, NULL);  }
	for (i=0; i<num_nn; i++)
	{  process_create(REACTION1_RUN, s_procs_completed_str, s_nn_str, s_n_str, NULL);  }
  	for (i=0; i<num_nn; i++)
	{  process_create(INJECTION1_RUN, s_procs_completed_str, s_nn_str, NULL);  }
  	for (i=1; i<=(num_hho); i++)
	{  
		if(i%2 == 0) process_create(INJECTION2_RUN, s_procs_completed_str, s_hho_str, NULL);
		else Printf("SKIPPING SIGNALLING ODD H2O (need multiples of 2).\n");
	}
    	// And finally, wait until all spawned processes have finished.
  	if (sem_wait(s_procs_completed) != SYNC_SUCCESS) 
	{  Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n"); Exit();  }

	// Done here, exiting
  	Printf("All producers/consumers have completed, exiting main process.\n");
	Printf("Final H2 and NO2 counts complete.\n");
}
