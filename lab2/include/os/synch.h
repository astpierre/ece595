///////////////////////////////////////////////////////////////////////////////
//	synch.h
//
//	Include file for synchronization stuff.  
//	The synchronization primitives include:
//	  > Semaphores
// 	  > Locks
//	  > Condition variables
//
//	Semaphores are the only "native" synchronization primitive.
//	Condition variables and locks are implemented using semaphores.
///////////////////////////////////////////////////////////////////////////////

#ifndef	_synch_h_
#define	_synch_h_

#include "queue.h"

#define SYNC_FAIL -1    // Used as return values from most synch functions.
#define SYNC_SUCCESS 1  // Note that many functions return handle (-1 for fail)

#define MAX_SEMS	32	//Maximum 32 semaphores allowed in the system
#define MAX_LOCKS	64	//Maximum 64 locks allowed in the system
				        //This is because cond vars use locks from same pool
#define MAX_CONDS	32	//Maximum 32 conds allowed in the system

/* TYPE DEF DECLARATIONS */
typedef int sem_t;
typedef int lock_t;
typedef int cond_t;

#define INVALID_SEM -1
#define INVALID_LOCK -1
#define INVALID_PROC -1
#define INVALID_COND -1

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------*/
/* SEMAPHORES */
typedef struct Sem {
    Queue	waiting;
    int		count;
    uint32	inuse; 		// Indicates whether the semaphore is being used
} Sem;

int SemInit(Sem *, int);
int SemWait(Sem *);
int SemSignal(Sem *);
/*---------------------------------------------------------------------------*/

/* LOCKS */
typedef struct Lock {
    int pid;            // PID of proc holding the lock, -1 if lock is avail
    Queue waiting;      // Queue of processes waiting on the lock
    int inuse;          // Bookkeeping variable for free vs. used structures
} Lock;

int LockInit(Lock *);
int LockAcquire(Lock *);
int LockRelease(Lock *);
/*---------------------------------------------------------------------------*/
/* CONDITION VARIABLES */
typedef struct Cond {
    int inuse;          // Bookkeeping variable for free vs. used structures
    Queue waiting;      // Queue of processes waiting on condvar
    lock_t lock;        // Associate lock handle passed from CondCreate
} Cond;

int CondInit(Cond *);
int CondWait(Cond *);
int CondSignal(Cond *);
/*---------------------------------------------------------------------------*/
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* SYNCHRONIZATION PRIMATIVES API FUNCTION HEADERS */
int SynchModuleInit();

sem_t SemCreate(int count);
int SemHandleWait(sem_t sem);
int SemHandleSignal(sem_t sem);
lock_t LockCreate();
int LockHandleAcquire(lock_t lock);
int LockHandleRelease(lock_t lock);
cond_t CondCreate(lock_t lock);
int CondHandleWait(cond_t cond);
int CondHandleSignal(cond_t cond);
int CondHandleBroadcast(cond_t cond);

#endif	//_synch_h_
