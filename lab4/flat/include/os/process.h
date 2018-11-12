//
//	process.h
//
//	Definitions for process creation and manipulation.  These include
//	the process control block (PCB) structure as well as information
//	about the stack format for a saved process.
//

#ifndef	__process_h__
#define	__process_h__

#include "dlxos.h"
#include "queue.h"

#define PROCESS_FAIL 0
#define PROCESS_SUCCESS 1

#define	PROCESS_MAX_PROCS	32	// Maximum number of active processes

#define	PROCESS_INIT_ISR_SYS	0x140	// Initial status reg value for system processes
#define	PROCESS_INIT_ISR_USER	0x100	// Initial status reg value for user processes

#define	PROCESS_STATUS_FREE	0x1
#define	PROCESS_STATUS_RUNNABLE	0x2
#define	PROCESS_STATUS_WAITING	0x4
#define	PROCESS_STATUS_STARTING	0x8
#define	PROCESS_STATUS_ZOMBIE	0x10
#define	PROCESS_STATUS_MASK	0x3f
#define	PROCESS_TYPE_SYSTEM	0x100
#define	PROCESS_TYPE_USER	0x200

typedef	void (*VoidFunc)();

// Process control block
typedef struct PCB {
  uint32	*currentSavedFrame; // -> current saved frame.  MUST BE 1ST!
  uint32	*sysStackPtr;	// Current system stack pointer.  MUST BE 2ND!
  uint32	sysStackArea;	// System stack area for this process
  unsigned int	flags;
  char		name[80];	// Process name
  uint32	pagetable[16];	// Statically allocated page table
  int		npages;		// Number of pages allocated to this process
  Link		*l;		// Used for keeping PCB in queues

  int           pinfo;          // Turns on printing of runtime stats
  int           pnice;          // Used in priority calculation
  int           runtime;        // Number of jiffies this program has run on CPU
  int           lasttime;       // Value of clock when process was switched in
  int           wakeuptime;     // Time (in jiffies) to wake a sleeping process
  int           sleeptime;      // Time (in jiffies) when a process was suspended
  int           autowake;       // Flag to indicate that ProcessSchedule can wake this up after wakeuptime is reached
  int           isyielding;     // Indicates that a PCB is yielding, so don't increment it's estcpu

  int           prio;           // Process's priority
  double        estcpu;         // Used in updating priority
  int           quanta_count;   // Number of full quanta that a process has run
  int           base_prio;      // Base priority (50 for user processes)

  int           isidle;         // Indicates if this PCB is the idle process
} PCB;

// Offsets of various registers from the stack pointer in the register
// save frame.  Offsets are in WORDS (4 byte chunks)
#define	PROCESS_STACK_IREG	10	// Offset of r0 (grows upwards)
// NOTE: r0 isn't actually stored!  This is for convenience - r1 is the
// first stored register, and is at location PROCESS_STACK_IREG+1
#define	PROCESS_STACK_FREG	(PROCESS_STACK_IREG+32)	// Offset of f0
#define	PROCESS_STACK_IAR	(PROCESS_STACK_FREG+32) // Offset of IAR
#define	PROCESS_STACK_ISR	(PROCESS_STACK_IAR+1)
#define	PROCESS_STACK_CAUSE	(PROCESS_STACK_IAR+2)
#define	PROCESS_STACK_FAULT	(PROCESS_STACK_IAR+3)
#define	PROCESS_STACK_PTBASE	(PROCESS_STACK_IAR+4)
#define	PROCESS_STACK_PTSIZE	(PROCESS_STACK_IAR+5)
#define	PROCESS_STACK_PTBITS	(PROCESS_STACK_IAR+6)
#define	PROCESS_STACK_PREV_FRAME 10	// points to previous interrupt frame
#define	PROCESS_STACK_FRAME_SIZE 85	// interrupt frame is 85 words
#define PROCESS_MAX_NAME_LENGTH  100    // Maximum length of an executable's filename
#define SIZE_ARG_BUFF  	1024		// Max number of characters in the
					// command-line arguments
#define MAX_ARGS	128		// Max number of command-line
					// arguments

#define PROCESS_MIN_USER_PRIORITY 50
#define PROCESS_MAX_PRIORITY 127
#define PROCESS_PRIORITIES_PER_QUEUE 4
#define PROCESS_NUM_QUEUES ((PROCESS_MAX_PRIORITY+1) / PROCESS_PRIORITIES_PER_QUEUE) // should be 32

// Number of jiffies in a single process quantum (i.e. how often ProcessSchedule is called)
#define PROCESS_QUANTUM_JIFFIES  CLOCK_PROCESS_JIFFIES
// Number of jiffies that have to pass before decaying all estcpu's
#define PROCESS_ESTCPU_DECAY_JIFFIES  ( PROCESS_QUANTUM_JIFFIES * 10 )

extern PCB	*currentPCB;

int ProcessFork (VoidFunc func, uint32 param, char *name, int isUser);
void ProcessSchedule ();
void ContextSwitch(void *, void *, int);
void ProcessSuspend (PCB *);
void ProcessSleep(); // A trap in dlxos.s, not a function
void ProcessWakeup (PCB *);
void ProcessSetResult (PCB *, uint32);
void ProcessUserSleep ();
void ProcessDestroy(PCB *pcb);
extern unsigned GetCurrentPid();
void process_create(char *name, ...);
int GetPidFromAddress(PCB *pcb);
void ProcessRecalcPriority(PCB *pcb);
inline int WhichQueue(PCB *pcb);
int ProcessInsertRunning(PCB *pcb);
void ProcessDecayEstcpu(PCB *pcb);
void ProcessDecayEstcpuSleep(PCB *pcb, int time_asleep_jiffies);
PCB *ProcessFindHighestPriorityPCB();
void ProcessDecayAllEstcpus();
void ProcessFixRunQueues();
void ProcessIdle();
void ProcessForkIdle();
int ProcessCountAutowake();
void ProcessPrintRunQueues();
void ProcessYield();

#endif	/* __process_h__ */
