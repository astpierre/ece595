//
//	synch.c
//
//	Routines for synchronization
//
//

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"

static Sem sems[MAX_SEMS]; 	        // All semaphores in the system
static Lock locks[MAX_LOCKS];       // All locks in the system
static Cond conds[MAX_CONDS];       // All cond vars in the system

extern struct PCB *currentPCB; 
//----------------------------------------------------------------------
//	SynchModuleInit
//
//	Initializes the synchronization primitives: the semaphores
//----------------------------------------------------------------------
int SynchModuleInit() {
    int i; // Loop Index variable
    dbprintf ('p', "SynchModuleInit: Entering SynchModuleInit\n");
    for(i=0; i<MAX_SEMS; i++) {
        sems[i].inuse = 0;
    }
    for(i=0; i<MAX_LOCKS; i++) {
        locks[i].inuse = 0;
        locks[i].pid = -1;
    }
    for(i=0; i<MAX_CONDS; i++) {
        conds[i].inuse = 0;
        conds[i].lock = -1;
    }
    dbprintf ('p', "SynchModuleInit: Leaving SynchModuleInit\n");
    return SYNC_SUCCESS;
}

//---------------------------------------------------------------------
//
//	SemInit
//
//	Initialize a semaphore to a particular value.  This just means
//	initting the process queue and setting the counter.
//
//----------------------------------------------------------------------
int SemInit (Sem *sem, int count) {
    if (!sem) return SYNC_FAIL;
    if (AQueueInit (&sem->waiting) != QUEUE_SUCCESS) {
        printf("FATAL ERROR: could not initialize semaphore waiting queue in SemInit!\n");
        exitsim();
    }
    sem->count = count;
    return SYNC_SUCCESS;
}

//----------------------------------------------------------------------
// 	SemCreate
//
//	Grabs a Semaphore, initializes it and returns a handle to this
//	semaphore. All subsequent accesses to this semaphore should be made
//	through this handle.  Returns SYNC_FAIL on failure.
//----------------------------------------------------------------------
sem_t SemCreate(int count) {
    sem_t sem;
    uint32 intrval;
    
    // grabbing a semaphore should be an atomic operation
    intrval = DisableIntrs();
    for(sem=0; sem<MAX_SEMS; sem++) {
        if(sems[sem].inuse==0) {
            sems[sem].inuse = 1;
            break;
        }
    }
    RestoreIntrs(intrval);
    if(sem==MAX_SEMS) return SYNC_FAIL;
    
    if (SemInit(&sems[sem], count) != SYNC_SUCCESS) return SYNC_FAIL;
    return sem;
}


//----------------------------------------------------------------------
//
//	SemWait
//
//	Wait on a semaphore.  As described in Section 6.4 of _OSC_,
//	we decrement the counter and suspend the process if the
//	semaphore's value is less than 0.  To ensure atomicity,
//	interrupts are disabled for the entire operation, but must be
//      turned on before going to sleep.
//
//----------------------------------------------------------------------
int SemWait (Sem *sem) {
  Link	*l;
  int		intrval;
    
  if (!sem) return SYNC_FAIL;

  intrval = DisableIntrs ();
  dbprintf ('I', "SemWait: Old interrupt value was 0x%x.\n", intrval);
  dbprintf ('s', "SemWait: Proc %d waiting on sem %d, count=%d.\n", GetCurrentPid(), (int)(sem-sems), sem->count);
  if (sem->count <= 0) {
    dbprintf('s', "SemWait: putting process %d to sleep\n", GetCurrentPid());
    if ((l = AQueueAllocLink ((void *)currentPCB)) == NULL) {
      printf("FATAL ERROR: could not allocate link for semaphore queue in SemWait!\n");
      exitsim();
    }
    if (AQueueInsertLast (&sem->waiting, l) != QUEUE_SUCCESS) {
      printf("FATAL ERROR: could not insert new link into semaphore waiting queue in SemWait!\n");
      exitsim();
    }
    ProcessSleep();
    // Don't decrement couter here because that's handled in SemSignal for us
  } else {
    sem->count--; // Decrement internal counter
    dbprintf('s', "SemWait: Proc %d granted permission to continue by sem %d\n", GetCurrentPid(), (int)(sem-sems));
  }
  RestoreIntrs (intrval);
  return SYNC_SUCCESS;
}

int SemHandleWait(sem_t sem) {
  if (sem < 0) return SYNC_FAIL;
  if (sem >= MAX_SEMS) return SYNC_FAIL;
  if (!sems[sem].inuse)    return SYNC_FAIL;
  return SemWait(&sems[sem]);
}

//----------------------------------------------------------------------
//
//	SemSignal
//
//	Signal on a semaphore.  Again, details are in Section 6.4 of
//	_OSC_.
//
//----------------------------------------------------------------------
int SemSignal (Sem *sem) {
  Link *l;
  int	intrs;
  PCB *pcb;

  if (!sem) return SYNC_FAIL;

  intrs = DisableIntrs ();
  dbprintf ('s', "SemSignal: Process %d Signalling on sem %d, count=%d.\n", GetCurrentPid(), (int)(sem-sems), sem->count);
  // Increment internal counter before checking value
  sem->count++;
  if (sem->count > 0) { // check if there is a process to wake up
    if (!AQueueEmpty(&sem->waiting)) { // there is a process to wake up
      l = AQueueFirst(&sem->waiting);
      pcb = (PCB *)AQueueObject(l);
      if (AQueueRemove(&l) != QUEUE_SUCCESS) { 
        printf("FATAL ERROR: could not remove link from semaphore queue in SemSignal!\n");
        exitsim();
      }
      dbprintf ('s', "SemSignal: Waking up PID %d.\n", (int)(GetPidFromAddress(pcb)));
      ProcessWakeup (pcb);
      // Decrement counter on behalf of woken up PCB
      sem->count--;
    }
  }
  RestoreIntrs (intrs);
  return SYNC_SUCCESS;
}

int SemHandleSignal(sem_t sem) {
  if (sem < 0) return SYNC_FAIL;
  if (sem >= MAX_SEMS) return SYNC_FAIL;
  if (!sems[sem].inuse)    return SYNC_FAIL;
  return SemSignal(&sems[sem]);
}

//-----------------------------------------------------------------------
//	LockCreate
//
//	LockCreate grabs a lock from the systeme-wide pool of locks and 
//	initializes it.
//	It also sets the inuse flag of the lock to indicate that the lock is
//	being used by a process. It returns a unique id for the lock. All the
//	references to the lock should be made through the returned id. The
//	process of grabbing the lock should be atomic.
//
//	If a new lock cannot be created, your implementation should return
//	INVALID_LOCK (see synch.h).
//-----------------------------------------------------------------------
lock_t LockCreate() {
  lock_t l;
  uint32 intrval;

  // grabbing a lock should be an atomic operation
  intrval = DisableIntrs();
  for(l=0; l<MAX_LOCKS; l++) {
    if(locks[l].inuse==0) {
      locks[l].inuse = 1;
      break;
    }
  }
  RestoreIntrs(intrval);
  if(l==MAX_LOCKS) return SYNC_FAIL;

  if (LockInit(&locks[l]) != SYNC_SUCCESS) return SYNC_FAIL;
  return l;
}

int LockInit(Lock *l) {
  if (!l) return SYNC_FAIL;
  if (AQueueInit (&l->waiting) != QUEUE_SUCCESS) {
    printf("FATAL ERROR: could not initialize lock waiting queue in LockInit!\n");
    exitsim();
  }
  l->pid = -1;
  return SYNC_SUCCESS;
}

//---------------------------------------------------------------------------
//	LockHandleAcquire
//
//	This routine acquires a lock given its handle. The handle must be a 
//	valid handle for this routine to succeed. In that case this routine 
//	returns SYNC_FAIL. Otherwise the routine returns SYNC_SUCCESS.
//
//---------------------------------------------------------------------------
int LockAcquire(Lock *k) {
  Link	*l;
  int		intrval;
    
  if (!k) return SYNC_FAIL;

  // Locks are atomic
  intrval = DisableIntrs ();
  dbprintf ('I', "LockAcquire: Old interrupt value was 0x%x.\n", intrval);

  // Check to see if the current process owns the lock
  if (k->pid == GetCurrentPid()) {
    dbprintf('s', "LockAcquire: Proc %d already owns lock %d\n", GetCurrentPid(), (int)(k-locks));
    RestoreIntrs(intrval);
    return SYNC_SUCCESS;
  }

  dbprintf ('s', "LockAcquire: Proc %d asking for lock %d.\n", GetCurrentPid(), (int)(k-locks));
  if (k->pid >= 0) { // Lock is already in use by another process
    dbprintf('s', "LockAcquire: putting process %d to sleep\n", GetCurrentPid());
    if ((l = AQueueAllocLink ((void *)currentPCB)) == NULL) {
      printf("FATAL ERROR: could not allocate link for lock queue in LockAcquire!\n");
      exitsim();
    }
    if (AQueueInsertLast (&k->waiting, l) != QUEUE_SUCCESS) {
      printf("FATAL ERROR: could not insert new link into lock waiting queue in LockAcquire!\n");
      exitsim();
    }
    ProcessSleep();
  } else {
    dbprintf('s', "LockAcquire: lock is available, assigning to proc %d\n", GetCurrentPid());
    k->pid = GetCurrentPid();
  }
  RestoreIntrs(intrval);
  return SYNC_SUCCESS;
}

int LockHandleAcquire(lock_t lock) {
  if (lock < 0) return SYNC_FAIL;
  if (lock >= MAX_LOCKS) return SYNC_FAIL;
  if (!locks[lock].inuse)    return SYNC_FAIL;
  return LockAcquire(&locks[lock]);
}

//---------------------------------------------------------------------------
//	LockHandleRelease
//
//	This procedure releases the unique lock described by the handle. It
//	first checks whether the lock is a valid lock. If not, it returns SYNC_FAIL.
//	If the lock is a valid lock, it should check whether the calling
//	process actually holds the lock. If not it returns SYNC_FAIL. Otherwise it
//	releases the lock, and returns SYNC_SUCCESS.
//---------------------------------------------------------------------------
int LockRelease(Lock *k) {
  Link *l;
  int	intrs;
  PCB *pcb;

  if (!k) return SYNC_FAIL;

  intrs = DisableIntrs ();
  dbprintf ('s', "LockRelease: Proc %d releasing lock %d.\n", GetCurrentPid(), (int)(k-locks));

  if (k->pid != GetCurrentPid()) {
    dbprintf('s', "LockRelease: Proc %d does not own lock %d.\n", GetCurrentPid(), (int)(k-locks));
    return SYNC_FAIL;
  }
  k->pid = -1;
  if (!AQueueEmpty(&k->waiting)) { // there is a process to wake up
    l = AQueueFirst(&k->waiting);
    pcb = (PCB *)AQueueObject(l);
    if (AQueueRemove(&l) != QUEUE_SUCCESS) { 
      printf("FATAL ERROR: could not remove link from lock queue in LockRelease!\n");
      exitsim();
    }
    dbprintf ('s', "LockRelease: Waking up PID %d, assigning lock.\n", (int)(GetPidFromAddress(pcb)));
    k->pid = GetPidFromAddress(pcb);
    ProcessWakeup (pcb);
  }
  RestoreIntrs (intrs);
  return SYNC_SUCCESS;
}

int LockHandleRelease(lock_t lock) {
  if (lock < 0) return SYNC_FAIL;
  if (lock >= MAX_LOCKS) return SYNC_FAIL;
  if (!locks[lock].inuse)    return SYNC_FAIL;
  return LockRelease(&locks[lock]);
}

//--------------------------------------------------------------------------
//	CondCreate
//--------------------------------------------------------------------------
/*--------------------------------------------------------------------------
//	This function grabs a condition variable from the system-wide pool of
//	condition variables and associates the specified lock with
//	it. It should also initialize all the fields that need to initialized.
//	The lock being associated should be a valid lock, which means that
//	it should have been obtained via previous call to LockCreate(). 
//	
//	If for some reason a condition variable cannot be created (no more
//	condition variables left, or the specified lock is not a valid lock),
//	this function should return INVALID_COND (see synch.h). Otherwise it
//	should return handle of the condition variable.
//--------------------------------------------------------------------------*/
int CondInit(Cond *cond) {
    // Check cond valid
    if(!cond) return SYNC_FAIL;
         
    // Initialize a queue for waiting processes
    if(AQueueInit(&cond->waiting) != QUEUE_SUCCESS) {
        printf("FATAL ERROR: could not initialize condition variable waiting queue in CondCreate\n");
        exitsim();
    }

    // SUCCESSFUL INIT!
    return SYNC_SUCCESS;
}
cond_t CondCreate(lock_t lock) 
{
    cond_t cond;
    uint32 intrval;
    
    // Check that the lock we were given is valid
    if(lock < 0) return INVALID_COND;
    if(lock >= MAX_LOCKS) return INVALID_COND;
    if(!locks[lock].inuse) return INVALID_COND;

    // Atomically grab a condition variable
    intrval = DisableIntrs();
    for(cond=0; cond<MAX_CONDS; cond++) {
        // grab first unused condition variable from pool
        if(conds[cond].inuse==0) {
            conds[cond].lock = lock;
            conds[cond].inuse = 1;
            break;
        }
    }
    // Become !Atomic
    RestoreIntrs(intrval);

    // Check to make sure we found one
    if(cond==MAX_CONDS) return INVALID_COND;

    // Initialize!
    if(CondInit(&conds[cond]) != SYNC_SUCCESS) return SYNC_FAIL; 
   
    // Return the handle to the condition variable
    return cond;
}

//---------------------------------------------------------------------------
//	CondHandleWait
//---------------------------------------------------------------------------
/*---------------------------------------------------------------------------
//	This function makes the calling process block on the condition variable
//	till either ConditionHandleSignal or ConditionHandleBroadcast is
//	received. The process calling CondHandleWait must have acquired the
//	lock associated with the condition variable (the lock that was passed
//	to CondCreate. This implies the lock handle needs to be stored
//	somewhere. hint! hint!) for this function to
//	succeed. If the calling process has not acquired the lock, it does not
//	block on the condition variable, but a value of 1 is returned
//	indicating that the call was not successful. Return value of 0 implies
//	that the call was successful.
//
//	This function should be written in such a way that the calling process
//	should release the lock associated with this condition variable before
//	going to sleep, so that the process that intends to signal this
//	process could acquire the lock for that purpose. After waking up, the
//	blocked process should acquire (i.e. wait on) the lock associated with
//	the condition variable. In other words, this process does not
//	"actually" wake up until the process calling CondHandleSignal or
//	CondHandleBroadcast releases the lock explicitly.
//---------------------------------------------------------------------------*/
int CondWait(Cond *cond) {
    Link *l;
    int intrval;
    
    // Disable interrupts (BECOME ATOMIC)
    intrval = DisableIntrs();
    
    // Check the calling process actually has the lock
    if(locks[cond->lock].pid != GetCurrentPid()) {  RestoreIntrs(intrval); return SYNC_FAIL;  }

    // Grab a link for the process (queuing format)
    if((l=AQueueAllocLink ((void *)currentPCB)) == NULL) 
    {  printf("FATAL ERROR: could not alocate link for cvar queue in CondHandleWait.\n"); exitsim(); }

    // Insert link in queue
    if(AQueueInsertLast(&cond->waiting, l) != QUEUE_SUCCESS)
    {  printf("FATAL ERROR: could not insert new link in cvar queue in CondHandleWait.\n"); exitsim(); }

    // Release the lock to not be "busy waiting"
    if(LockHandleRelease(cond->lock) != SYNC_SUCCESS) return 0;
    
    // Restore process interrupts (BECOME !ATOMIC)
    RestoreIntrs(intrval);
    
    // Place process to sleep
    ProcessSleep();
    
    // Process has been awakened! Get the lock back
    if(LockHandleAcquire(cond->lock) != SYNC_SUCCESS) return 0;

    // Condition variable handled wait call successfully!
    return SYNC_SUCCESS;
}

int CondHandleWait(cond_t c) {       
    // Check condition variable handle is valid
    if(c < 0) return SYNC_FAIL;
    if(c >= MAX_CONDS) return SYNC_FAIL;
    if(!conds[c].inuse) return SYNC_FAIL;

    // Send to worker and return result
    return CondWait(&conds[c]);
}

//---------------------------------------------------------------------------
//	CondHandleSignal
//---------------------------------------------------------------------------
/*---------------------------------------------------------------------------
//	This call wakes up exactly one process waiting on the condition
//	variable, if at least one is waiting. If there are no processes
//	waiting on the condition variable, it does nothing. In either case,
//	the calling process must have acquired the lock associated with
//	condition variable for this call to succeed, in which case it returns
//	0. If the calling process does not own the lock, it returns 1,
//	indicating that the call was not successful. This function should be
//	written in such a way that the calling process should retain the
//	acquired lock even after the call completion (in other words, it
//	should not release the lock it has already acquired before the call).
//
//	Note that the process woken up by this call tries to acquire the lock
//	associated with the condition variable as soon as it wakes up. Thus,
//	for such a process to run, the process invoking CondHandleSignal
//	must explicitly release the lock after the call is complete.
//---------------------------------------------------------------------------*/
int CondSignal(Cond *cond) {
    Link *l;
    int intrval;
    PCB *pcb;
    
    if(!cond) return SYNC_FAIL;

    // Disable interrupts (BECOME ATOMIC)
    intrval = DisableIntrs();
    
    // Check the calling process actually has the lock
    if(locks[cond->lock].inuse != 1) {  RestoreIntrs(intrval); return SYNC_FAIL;  }

    // Check the queue to see if there is anyone to wake up... & if so, who?
    if(!AQueueEmpty(&cond->waiting)) {          
        // Get first process in line
        l = AQueueFirst(&cond->waiting);
        
        // Retrieve process control block of process
        pcb = (PCB *)AQueueObject(l);

        // Remove from condition variable queue
        if(AQueueRemove(&l) != QUEUE_SUCCESS) {  printf("FATAL ERROR: !rmv lnk frm condvar Q in CndHndSig\n"); exitsim();  }
        
        // Wakeup! HOWEVER: do not release lock... (MESA STYLE) 
        // Mesa Style requires calling process to keep key and release l8r
        ProcessWakeup(pcb);
    }

    // Restore interrupts (BECOME !ATOMIC)
    RestoreIntrs(intrval);

    // Condition variable handled signal call successfully!
    return SYNC_SUCCESS;
}
int CondHandleSignal(cond_t c) {
    // Check condition variable handle is valid
    if(c < 0) return SYNC_FAIL;
    if(c >= MAX_CONDS) return SYNC_FAIL;
    if(!conds[c].inuse) return SYNC_FAIL;

    // Send to worker and return result
    return CondSignal(&conds[c]);
}

//---------------------------------------------------------------------------
//	CondHandleBroadcast
//---------------------------------------------------------------------------
/*---------------------------------------------------------------------------
//	This function is very similar to CondHandleSignal. But instead of
//	waking only one process, it wakes up all the processes waiting on the
//	condition variable. For this call to succeed, the calling process must
//	have acquired the lock associated with the condition variable. This
//	function should be written in such a way that the calling process
//	should retain the lock even after call completion.
//
//	Note that the process woken up by this call tries to acquire the lock
//	associated with the condition variable as soon as it wakes up. Thus,
//	for such a process to run, the process invoking CondHandleBroadcast
//	must explicitly release the lock after the call completion.
//---------------------------------------------------------------------------*/ 
int CondHandleBroadcast(cond_t c) {  
    // Your code goes here
    return SYNC_SUCCESS;
}
