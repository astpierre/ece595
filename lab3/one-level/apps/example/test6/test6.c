#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int i = 0;

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Print a message
  Printf(" TEST6 (%d): START\n", getpid());

  // Count to a large number in a for loop
  while(i<1000000){  i++;  }

  // Print a message
  Printf(" TEST6 (%d): END\n", getpid());
 
  // Signal the semaphore to tell the original process that we're done
  sem_signal(s_procs_completed);
}
