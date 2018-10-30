#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Access memory beyond max virtual address
  Printf(" TEST2 (%d): Size of virtual memory       | = 1024 KB    |  = 1,048,576 bytes  |\n", getpid());
  Printf(" TEST2 (%d): Max virtual memory address   | = (1<<20)-1  |  = 0xFFFF in hex    |\n", getpid());
  Printf(" TEST2 (%d): Attempting to access address | = 0x10001    |\n", getpid());
  
  // Signal the semaphore to tell the original process that we're done
  sem_signal(s_procs_completed);

  Printf(" TEST2 (%d): Value at 0x10001 = %d\n", getpid(), *(unsigned int*)0x10001);

}
