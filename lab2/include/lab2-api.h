/*****************************************************************
 *
 * Various APIs for Lab 2 of OS469
 * Copyright: Uday Savagaonkar, Jan 2002
 *
 *****************************************************************/

#ifndef _LAB2_API_H_
#define _LAB2_API_H_

#define NULL (void *)0

#define SYNC_SUCCESS 1
#define SYNC_FAIL    -1

#define BUFFERSIZE       5

typedef unsigned int uint32;
typedef int sem_t;
typedef int lock_t;
typedef int cond_t;

//Related to processes
void process_create(char *arg1, ...);   //trap 0x432

//Related to shared memory
uint32 shmget();			        //trap 0x440
void *shmat(uint32 handle);		    //trap 0x441

//Related to semaphores
sem_t sem_create(int count);	    //trap 0x450
int sem_wait(sem_t sem);		    //trap 0x451
int sem_signal(sem_t sem);		    //trap 0x452

//-----------------------------------------------------------------------
// You need to write OS handler routines for the following functions
// We have already provided the trap interface. All you need to do is fill in
// the missing parts in synch.c and synch.h. The routines that need to be
// modified are also given in front of each system call. For detailed
// description of thses routines, see synch.c.
//------------------------------------------------------------------------

lock_t lock_create();			    //trap 0x452 Calls LockCreate()
int lock_acquire(lock_t lock);		//trap 0x453 Calls LockHandleAcquire()
int lock_release(lock_t lock);		//trap 0x454 Calls LockHandleRelease()

cond_t cond_create(lock_t lock);	//trap 0x455 Calls CondCreate()
int cond_wait(cond_t cond);		    //trap 0x456 Calls CondHandleWait()
int cond_signal(cond_t cond);		//trap 0x457 Calls CondHandleSignal()
int cond_broadcast(cond_t cond);	//trap 0x458 Calls CondHandleBroadcast()

#endif _LAB2_API_H_

