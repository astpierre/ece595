//
//	synch.h
//
//	Include file for synchronization stuff.  The synchronization
//	primitives include:
//	Semaphore
//	Lock
//	Condition
//
//	Semaphores are the only "native" synchronization primitive.
//	Condition variables and locks are implemented using semaphores.
//

#ifndef	_synch_h_
#define	_synch_h_

#include "queue.h"

#define SYNC_FAIL -1    // Used as return values from most synchronization functions.
#define SYNC_SUCCESS 1  // Note that many functions return a handle, hence the -1 value
                        // for failure.

#define MAX_SEMS	32	//Maximum 32 semaphores allowed in the system
#define MAX_LOCKS	64	//Maximum 64 locks allowed in the system
				//This is because condition vars also use
				//locks from the same pool
#define MAX_CONDS	32	//Maximum 32 conds allowed in the system

typedef int sem_t;
typedef int lock_t;
typedef int cond_t;

#define INVALID_SEM -1
#define INVALID_LOCK -1
#define INVALID_PROC -1
#define INVALID_COND -1
typedef struct Sem {
    Queue	waiting;
    int		count;
    uint32	inuse; 		//indicates whether the semaphore is being
    				//used by any process
} Sem;

int SemInit (Sem *, int);
int SemWait (Sem *);
int SemSignal (Sem *);

typedef struct Lock {
  int pid;       // PID of process holding the lock, -1 if lock is available
  Queue waiting; // Queue of processes waiting on the lock
  int inuse;     // Bookkeeping variable for free vs. used structures
} Lock;

int LockInit(Lock *);
int LockAcquire(Lock *);
int LockRelease(Lock *);

typedef struct Cond {
  Lock *lock;    // Lock associated with this conditional variable
  Queue waiting; // Queue of processes waiting on the conditional variable 
  int inuse;     // Bookkeeping variable for free vs. used structures
} Cond;

int CondInit(Cond *);
int CondWait(Cond *);
int CondSignal(Cond *);

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
