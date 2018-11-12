#ifndef __usertraps_h__
#define __usertraps_h__

//---------------------------------------------------------------------
// Typedefs needed for user-level traps
//---------------------------------------------------------------------
typedef int sem_t;
typedef int lock_t;
typedef int cond_t;
typedef int mbox_t;

//---------------------------------------------------------------------
// Any #defines from operating system for return values
//---------------------------------------------------------------------
#define DISK_FAIL -1
#define DISK_SUCCESS 1
#define SYNC_FAIL -1
#define SYNC_SUCCESS 1

//---------------------------------------------------------------------
// Function declarations for user traps defined in the assembly 
// library are provided here.  User-level programs can use any of 
// these functions, and any new traps need to have their declarations
// listed here.
//---------------------------------------------------------------------

int Open(char *filename, int arg2);
void Printf(char *format, ...);
void Exit();

// Related to processes
int getpid();                           //trap 0x431
void process_create(char *exec_name, int pnice, int pinfo, ...);  //trap 0x432

// Related to semaphores
sem_t sem_create(int count);		//trap 0x450
int sem_wait(sem_t sem);		//trap 0x451
int sem_signal(sem_t sem);		//trap 0x452

// Related to locks
lock_t lock_create();			//trap 0x453
int lock_acquire(lock_t lock);		//trap 0x454
int lock_release(lock_t lock);		//trap 0x455

// Related to conditional variables
cond_t cond_create(lock_t lock);	//trap 0x456
int cond_wait(cond_t cond);		//trap 0x457
int cond_signal(cond_t cond);		//trap 0x458
int cond_broadcast(cond_t cond);	//trap 0x459

// Related to the disk
int disk_write_block(int blocknum, char *b); //trap 0x467
int disk_size();                        //trap 0x468
int disk_blocksize();                   //trap 0x469
int disk_create();                      //trap 0x470

// Related to DFS file system
void dfs_invalidate();                  //trap 0x471

// Related to files
unsigned int file_open(char *filename, char *mode);
int file_close(unsigned int handle);
int file_delete(char *filename);
int file_read(unsigned int handle, void *mem, int num_bytes);
int file_write(unsigned int handle, void *mem, int num_bytes);
int file_seek(unsigned int handle, int num_bytes, int from_where);



// Miscellaneous traps
void run_os_tests();

#ifndef NULL
#define NULL (void *)0x0
#endif
#endif
