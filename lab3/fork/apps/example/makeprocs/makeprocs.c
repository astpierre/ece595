#include "usertraps.h"
#include "misc.h"

#define TEST1 "hello_world.dlx.obj"
#define TEST2 "test2.dlx.obj"
#define TEST3 "test3.dlx.obj"
#define TEST4 "test4.dlx.obj"
#define TEST5 "test5.dlx.obj"
#define TEST6 "test6.dlx.obj"V

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * FUNCTION:        makeprocs.c - user test program for fork() system call
 *
 * DESCRIPTION:     This program is the controller for test programs.
 *                  Upon execution, this program will launch each test in
 *                  succession. The test sequence and purposes are described
 *                  below. This function has no input arguments.
 *
 * TEST1:           Testing the progress of the fork() sys call
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void main (int argc, char *argv[])
{
    int i;                               // Loop index variable
    int variable = 123;                  // Variable used to test fork()
    int child = 0;                       // Child spawned process

    // CHECK INPUT ARGUMENTS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (argc != 1) 
    {  Printf("Usage: %s\n", argv[0]); Exit();  } 

    Printf("================================================================================\n");
    Printf(" parent (%d): I am the controlling process for this user test program\n", getpid());
    Printf(" parent (%d): About to launch tests demonstrating proper fork() copy-on-write\n", getpid());
    Printf("================================================================================\n");
    
    // LAUNCH TEST1 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Printf("\n");
    Printf("\n");
    Printf("================================================================================\n");
    Printf(" parent (%d): Test 1 is a progress checker on the fork() syscall\n",getpid());
    Printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");
 
    Printf(" parent (%d): variable = (%d)\n",getpid(),variable);
    Printf(" parent (%d): calling fork()\n",getpid());
    child = fork();
    if(child != 0)
    {
        // PARENT PROCESS
        Printf(" p (%d): after calling fork()\n",getpid());
        Printf(" p (%d): variable = (%d)\n",getpid(),variable);
    }
    else
    {
        // CHILD PROCESS
        Printf("  child (%d): hello, I am child\n",getpid());
        Printf("  child (%d): variable = (%d)\n",getpid(),variable);
    }

    // End of test
    //Printf("________________________________________________________________________________\n");
    //Printf(" parent (%d): Test 1 complete\n",getpid());
    //Printf("================================================================================\n");

    //Printf("================================================================================\n");
    //Printf("================================================================================\n");
    //Printf(" parent (%d): All other processes completed, exiting main process\n", getpid());
    //Printf("================================================================================\n");
}
