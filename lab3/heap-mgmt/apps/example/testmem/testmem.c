#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
    sem_t s_procs_completed; // Semaphore to signal the original process that we're done
    int * mem_1_ptr;
    int * mem_2_ptr;
    int * mem_3_ptr;
    int freed_count;

    // Check input args...
    if (argc != 2) 
    {  Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); Exit();  } 

    // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);

    // Perform Mallocs...
    Printf(" testmem (%d): About to perform mem alloc. Unless noted, messages are from OS.\n\n", getpid());
    //-----------------------
    // Malloc 512 bytes
    mem_1_ptr = malloc(512);
    // Malloc 1024 bytes
    mem_2_ptr = malloc(512);
    // Malloc 2048 bytes
    mem_3_ptr = malloc(512);


    // Free chunk
    Printf(" testmem (%d): Expected= (512)bytes freed...   \n\n", getpid());
    freed_count=mfree(mem_1_ptr);
    Printf(" testmem (%d): Actual= (%d)bytes freed\n\n",getpid(), freed_count);

    // Free chunk
    Printf(" testmem (%d): Expected= (512)bytes freed...   \n\n", getpid());
    freed_count=mfree(mem_2_ptr);
    Printf(" testmem (%d): Actual= (%d)bytes freed\n\n",getpid(),freed_count);
    
    // Free chunk
    Printf(" testmem (%d): Expected= (512)bytes freed...  \n\n", getpid());
    freed_count=mfree(mem_3_ptr);
    Printf(" testmem (%d): Actual= (%d)bytes freed\n\n",getpid(), freed_count);

    // Signal the semaphore to tell the original process that we're done
    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) 
    {  Printf("TEST1 (%d): Bad semaphore!\n", getpid()); Exit();  }
}
