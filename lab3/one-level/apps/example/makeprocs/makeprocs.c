#include "usertraps.h"
#include "misc.h"

#define TEST1 "hello_world.dlx.obj"
#define TEST2 "test2.dlx.obj"
#define TEST3 "test3.dlx.obj"
#define TEST4 "test4.dlx.obj"
#define TEST5 "test5.dlx.obj"
#define TEST6 "test6.dlx.obj"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * FUNCTION:        makeprocs.c - user test program for one-level paging
 *
 * DESCRIPTION:     This program is the controller for 6 test programs.
 *                  Upon execution, this program will launch each test in
 *                  succession. The test sequence and purposes are described
 *                  below. This function has no input arguments.
 *
 * TEST1:           Print "Hello World" and exit
 * 
 * TEST2:           Access memory beyond the maximum virtual address
 * 
 * TEST3:           Access memory inside the maximum virtual address but 
 *                  outside of currently allocated pages
 * 
 * TEST4:           Cause a user function call stack to grow larger than 1 
 *                  page
 *
 * TEST5:           Call the "Hello World" program 100 times to make sure
 *                  pages are being properly allocated and freed
 *
 * TEST6:           Spawn 30 simultaneous processes that print a message, 
 *                  count to a large number in a for loop, and then print
 *                  another message before exiting. The number chosen must be
 *                  large enough such that all 30 processes are running at 
 *                  the same time and memory does not run out
 *
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
    Printf(" makeprocs (%d): About to launch six tests demonstrating proper one-level paging\n", getpid());
    Printf("================================================================================\n");
    
    // LAUNCH TEST1 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Printf("\n");
    Printf("\n");
    Printf("================================================================================\n");
    Printf(" makeprocs (%d): Test 1 prints \"Hello World\" and exits\n",getpid());
    Printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");
    // Initialize semaphore
    if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) 
    {  Printf("makeprocs (%d): Bad sem_create\n", getpid()); Exit();  }
    
    // Setup CLA
    ditoa(s_procs_completed, s_procs_completed_str);

    // Create process
    process_create(TEST1, s_procs_completed_str, NULL);

    // Wait for signal
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) 
    {  Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]); Exit();  }

    // End of test
    Printf("________________________________________________________________________________\n");
    Printf(" makeprocs (%d): Test 1 complete\n",getpid());
    Printf("================================================================================\n");

    // LAUNCH TEST2 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Printf("\n");
    Printf("\n");
    Printf("================================================================================\n");
    Printf(" makeprocs (%d): Test 2 tries accessing memory beyond the max virtual address\n",getpid());
    Printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");
    // Initialize semaphore
    if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) 
    {  Printf("makeprocs (%d): Bad sem_create\n", getpid()); Exit();  }
    
    // Setup CLA
    ditoa(s_procs_completed, s_procs_completed_str);

    // Create process
    process_create(TEST2, s_procs_completed_str, NULL);

    // Wait for signal
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) 
    {  Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]); Exit();  }

    // End of test
    Printf("________________________________________________________________________________\n");
    Printf(" makeprocs (%d): Test 2 complete\n",getpid());
    Printf("================================================================================\n");
    
    // LAUNCH TEST3 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Printf("\n");
    Printf("\n");
    Printf("================================================================================\n");
    Printf(" makeprocs (%d): Test 3 tries to access mem in vaddr, but not in allocated page\n",getpid());
    Printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");
    // Initialize semaphore
    if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) 
    {  Printf("makeprocs (%d): Bad sem_create\n", getpid()); Exit();  }
    
    // Setup CLA
    ditoa(s_procs_completed, s_procs_completed_str);

    // Create process
    process_create(TEST3, s_procs_completed_str, NULL);

    // Wait for signal
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) 
    {  Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]); Exit();  }

    // End of test
    Printf("________________________________________________________________________________\n");
    Printf(" makeprocs (%d): Test 3 complete\n",getpid());
    Printf("================================================================================\n");
    
    // LAUNCH TEST4 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Printf("\n");
    Printf("\n");
    Printf("================================================================================\n");
    Printf(" makeprocs (%d): Test 4 calls a recursive func until call stack needs a new page\n",getpid());
    Printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");
    // Initialize semaphore
    if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) 
    {  Printf("makeprocs (%d): Bad sem_create\n", getpid()); Exit();  }
    
    // Setup CLA
    ditoa(s_procs_completed, s_procs_completed_str);

    // Create process
    process_create(TEST4, s_procs_completed_str, NULL);

    // Wait for signal
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) 
    {  Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]); Exit();  }

    // End of test
    Printf("________________________________________________________________________________\n");
    Printf(" makeprocs (%d): Test 4 complete\n",getpid());
    Printf("================================================================================\n");
    
    // LAUNCH TEST5 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Printf("\n");
    Printf("\n");
    Printf("================================================================================\n");
    Printf(" makeprocs (%d): Test 5 prints \"Hello World\" 100 times\n",getpid());
    Printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");
    
    // Initialize semaphore
    if ((s_procs_completed = sem_create(-99)) == SYNC_FAIL) 
    {  Printf("makeprocs (%d): Bad sem_create\n", getpid()); Exit();  }
    // Setup CLA
    ditoa(s_procs_completed, s_procs_completed_str);
    
    // Initialize loop variable
    i=0;
    // Create processes
    while(i<100)
    {
        // Create process and increment counter
        process_create(TEST1, s_procs_completed_str, NULL);
        i++;
    }
    
    // Wait for signal
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) 
    {  Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]); Exit();  }

    // End of test
    Printf("________________________________________________________________________________\n");
    Printf(" makeprocs (%d): Test 5 complete\n",getpid());
    Printf("================================================================================\n");
    
    // LAUNCH TEST6 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Printf("\n");
    Printf("\n");
    Printf("================================================================================\n");
    Printf(" makeprocs (%d): Test 6 creates 30 long simulaneously running procs\n",getpid());
    Printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");
    // Initialize semaphore
    if ((s_procs_completed = sem_create(-29)) == SYNC_FAIL) 
    {  Printf("makeprocs (%d): Bad sem_create\n", getpid()); Exit();  }
    
    // Setup CLA
    ditoa(s_procs_completed, s_procs_completed_str);

    // Initialize loop variable
    i=0;
    // Create processes
    while(i<30)
    {
        // Create process and increment counter
        process_create(TEST6, s_procs_completed_str, NULL);
        i++;
    }

    // Wait for signal
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) 
    {  Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]); Exit();  }

    // End of test
    Printf("________________________________________________________________________________\n");
    Printf(" makeprocs (%d): Test 6 complete\n",getpid());
    Printf("================================================================================\n");
   

    Printf("================================================================================\n");
    Printf("================================================================================\n");
    Printf(" makeprocs (%d): All other processes completed, exiting main process\n", getpid());
    Printf("================================================================================\n");
}
