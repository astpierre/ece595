//
// dlxtraps.h
//
// Traps used by the DLX simulator.  These traps have to be handled by the
// operating system.
//
// Copyright 1999 by Ethan L. Miller
// University of Maryland Baltimore County
//
// $Id: traps.h,v 1.1 2000/09/20 01:50:19 elm Exp elm $
//

#ifndef	_dlxtraps_h_
#define	_dlxtraps_h_

// Non-trap #defines:
#define PRINTF_MAX_FORMAT_LENGTH 256       // Maximum length of any Printf format string
#define PRINTF_MAX_ARGS 8                  // Maximum number of "%"'s in a Printf format string 
                                           //    (note that one %f is actually 2 args worth of space)
                                           //    Also, you cannot make this greater than 8.  It seems that the
                                           //    the compiler will only pass up to 8 parameters on the stack
                                           //    (9 including the format string).  So, you'll get random values
                                           //    for any more than 8 args.
#define PRINTF_MAX_STRING_ARGS 5           // Maximum number of "%s"'s allowed in a format string
#define PRINTF_MAX_STRING_ARG_LENGTH 100   // Maximum length of any string printed with a "%s"

// Traps:

#define	TRAP_ILLEGALINST	0x1	// Illegal instruction
#define	TRAP_ADDRESS		0x2	// Bad address
#define	TRAP_ACCESS		0x3	// Attempted to access illegal memory
#define	TRAP_OVERFLOW		0x4	// Math overflow
#define	TRAP_DIV0		0x5	// Divide by 0
#define	TRAP_PRIVILEGE		0x6	// Instruction must be executed as sys
#define	TRAP_FORMAT		0x7	// Instruction is malformed
#define	TRAP_PAGEFAULT		0x20
#define	TRAP_TLBFAULT		0x30
#define	TRAP_TIMER		0x40	// timer interrupt
#define	TRAP_KBD		0x48	// keyboard interrupt

// This bit is set in CAUSE if the interrupt was a trap instruction
#define	TRAP_TRAP_INSTR		0x08000000

// The following traps are all defined in the basic traps library.  To
// call TRAP_READ, use a Read() call from a user-level program.  This applies
// to every trap except EXIT, which is called via exit() [no uppercase].
#define	TRAP_PRINTF		0x201
#define	TRAP_READ		0x210
#define	TRAP_WRITE		0x211
#define	TRAP_LSEEK		0x212
#define	TRAP_SEEK		TRAP_LSEEK	// SEEK and LSEEK are synonyms
#define	TRAP_OPEN		0x213
#define	TRAP_CLOSE		0x214
#define	TRAP_DELETE		0x580
#define	TRAP_EXIT		0x300

// Following are user-defined traps.  Traps should be in the range
// 0x400 - 0xfff
#define	TRAP_CONTEXT_SWITCH	0x400
#define	TRAP_PROCESS_SLEEP	0x410
#define	TRAP_PROCESS_WAKEUP	0x420
#define	TRAP_PROCESS_FORK	0x430
#define TRAP_PROCESS_GETPID	0x431
#define TRAP_PROCESS_CREATE	0x432
#define TRAP_SHARE_CREATE_PAGE	0x440
#define TRAP_SHARE_MAP_PAGE	0x441
#define TRAP_SEM_CREATE		0x450
#define TRAP_SEM_WAIT		0x451
#define TRAP_SEM_SIGNAL		0x452
#define TRAP_LOCK_CREATE	0x453
#define TRAP_LOCK_ACQUIRE	0x454
#define TRAP_LOCK_RELEASE	0x455
#define TRAP_COND_CREATE	0x456
#define TRAP_COND_WAIT		0x457
#define TRAP_COND_SIGNAL	0x458
#define TRAP_COND_BROADCAST	0x459
#define TRAP_MBOX_CREATE        0x460
#define TRAP_MBOX_OPEN          0x461
#define TRAP_MBOX_CLOSE         0x462
#define TRAP_MBOX_SEND          0x463
#define TRAP_MBOX_RECV          0x464
#define TRAP_USER_SLEEP         0x465
#define TRAP_YIELD              0x466
#define TRAP_MALLOC             0x467
#define TRAP_MFREE              0x468

#define TRAP_USER_EXIT          0x500

// The following are special I/O addresses for DLX.
#define	DLX_TIMER_ADDRESS	0xfff00010
#define	DLX_KBD_PUTCHAR		0xfff00100
#define	DLX_KBD_NCHARSOUT	0xfff00120
#define	DLX_KBD_GETCHAR		0xfff00180
#define	DLX_KBD_NCHARSIN	0xfff001a0
#define	DLX_KBD_INTR		0xfff001c0

#define	TRAP_STACK_SIZE		0x800	// interrupt stack is 2K words

#endif	/* _dlxtraps_h_ */
