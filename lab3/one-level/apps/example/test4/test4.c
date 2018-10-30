#include "usertraps.h"
#include "misc.h"

#define MEM_MAX_VIRTUAL_ADDRESS (1<<20)-1
#define MEM_PAGESIZE (0x1<<12)

// Global variable used to compare pages recursively
int firstPage = -1;
int callStackGrower(int val)
{
    // Grab the address of the argument being passed
    int val_addr = &val;

    Printf(" TEST4 (%d) callStackGrower: run (%d), page number (%d)\n",getpid(),val,(val_addr>>12));
    if(firstPage == -1) firstPage = val_addr >> 12;
    else if((val_addr>>12) != firstPage) 
    {
        Printf(" TEST4 (%d) callStackGrower: first page (%d), new page (%d)\n",getpid(),firstPage,(val_addr>>12));
        return 0;
    }
    return callStackGrower(val+1);
}


void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int variable = 1;

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Access memory within max virtual address, but outside of allocated pages
  Printf(" TEST4 (%d): Size of a page is 4KB\n", getpid());
  Printf(" TEST4 (%d): This test will call callStackGrower(), a recursive function\n", getpid());
  Printf(" TEST4 (%d): The terminating condition is the page changing\n", getpid());
  callStackGrower(variable);
  
  // Signal the semaphore to tell the original process that we're done
  sem_signal(s_procs_completed);
}
