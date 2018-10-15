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
#define MBOX_FAIL -1
#define MBOX_SUCCESS 1
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
void process_create(char *exec_name, ...);  //trap 0x432

// Related to shared memory
unsigned int shmget();			//trap 0x440
void *shmat(unsigned int handle);	//trap 0x441

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

// Related to mailboxes
mbox_t mbox_create();                   //trap 0x460
int mbox_open(mbox_t handle);           //trap 0x461
int mbox_close(mbox_t handle);          //trap 0x462
int mbox_send(mbox_t handle, int length, void *data); // trap 0x463
int mbox_recv(mbox_t handle, int maxlength, void *data); // trap 0x464

// Related to process scheduling
void sleep(int seconds);                //trap 0x465
void yield();                           //trap 0x466

//Related to heap management
void *malloc(int memsize);              //trap 0x467
int mfree(void *ptr);                   //trap 0x468


#ifndef NULL
#define NULL (void *)0x0
#endif
#endif
