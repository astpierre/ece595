#include "usertraps.h"
#include "misc.h"

#define TESTMEM "testmem.dlx.obj"
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * FUNCTION:        makeprocs.c - user test program for sys calls malloc mfree
 *
 * DESCRIPTION:     This program is the controller for test programs.
 *                  Upon execution, this program will launch each test in
 *                  succession. The test sequence and purposes are described
 *                  below. This function has no input arguments.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void main (int argc, char *argv[])
{
    int i;                               // Loop index variable
    sem_t s_procs_completed;             // Semaphore used for flow control
    char s_procs_completed_str[10];      // Used as command-line argument 

    // CHECK INPUT ARGUMENTS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (argc != 1) 
    {  Printf("Usage: %s\n", argv[0]); Exit();  } 

    Printf("================================================================================\n");
    Printf(" makeprocs (%d): I am the controlling process for this user test program\n", getpid());
    Printf(" makeprocs (%d): About to launch a test demo'ing proper memory allocation / freeing\n", getpid());
    Printf("================================================================================\n");
    
    // LAUNCH TEST1 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Printf("\n");
    Printf("\n");
    Printf("================================================================================\n");
    Printf(" makeprocs (%d): Test memory will allocate memory of misc sizes and frees after\n",getpid());
    Printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");
    // Initialize semaphore
    if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) 
    {  Printf("makeprocs (%d): Bad sem_create\n", getpid()); Exit();  }
    
    // Setup CLA
    ditoa(s_procs_completed, s_procs_completed_str);

    // Create process
    process_create(TESTMEM,s_procs_completed_str, NULL);

    // Wait for signal
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) 
    {  Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]); Exit();  }

    // End of test
    Printf("________________________________________________________________________________\n");
    Printf(" makeprocs (%d): Test memory complete\n",getpid());
    Printf("================================================================================\n");

    Printf("================================================================================\n");
    Printf("================================================================================\n");
    Printf(" makeprocs (%d): All other processes completed, exiting main process\n", getpid());
    Printf("================================================================================\n");
}
