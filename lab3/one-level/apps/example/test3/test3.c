#include "usertraps.h"
#include "misc.h"

#define MEM_MAX_VIRTUAL_ADDRESS (1<<20)-1
#define MEM_PAGESIZE (0x1<<12)

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int * target = (int*)(MEM_MAX_VIRTUAL_ADDRESS+1-MEM_PAGESIZE)-4;

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Access memory within max virtual address, but outside of allocated pages
  Printf(" TEST3 (%d): Size of virtual memory       | = 1024 KB    |  = 1,048,576 bytes  |\n", getpid());
  Printf(" TEST3 (%d): Max virtual memory address   | = (1<<20)-1  |  = 0xFFFF in hex    |\n", getpid());
  Printf(" TEST3 (%d): User stack page is located at top of virtual memory\n", getpid());

  Printf(" TEST3 (%d): Attempting to access page right after user stack page\n", getpid());
  Printf(" TEST3 (%d): Target page = (0xFFFF + 1 - MEM_PAGESIZE) - 4\n", getpid());
  
  // Signal the semaphore to tell the original process that we're done
  sem_signal(s_procs_completed);

  Printf(" TEST3 (%d): Value = %d\n", getpid(),*target);
}
