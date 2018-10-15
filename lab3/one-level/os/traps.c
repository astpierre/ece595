//
//	traps.c
//
//	This file handles the low-level trap stuff for an operating system
//	running under DLX.

#include "dlx.h"
#include "dlxos.h"
#include "ostraps.h"
#include "traps.h"
#include "process.h"
#include "memory.h"
#include "synch.h"
#include "clock.h"


//----------------------------------------------------------------------
//
//	KbdModuleInit
//
//	Initialize the keyboard module.  This involves turning on
//	interrupts for the keyboard.
//
//----------------------------------------------------------------------
void
KbdModuleInit ()
{
  *((uint32 *)DLX_KBD_INTR) = 1;
}

//--------------------------------------------------------------------
// GetUintFromTrapArg(uint32 *trapArgs, int sysmode)
//--------------------------------------------------------------------
uint32 GetUintFromTrapArg(uint32 *trapArgs, int sysmode)
{
  uint32 arg;
  if(!sysmode)
  {
    MemoryCopyUserToSystem (currentPCB, (char *)trapArgs, (char *)&arg, sizeof (arg));
  } 
  else 
  {
    bcopy ((void *)trapArgs, (void *)&arg, sizeof (arg));
  }

  return arg;
}

//--------------------------------------------------------------------
// GetIntFromTrapArg(uint32 *trapArgs, int sysmode)
//--------------------------------------------------------------------
static int GetIntFromTrapArg(uint32 *trapArgs, int sysmode)
{
  int arg;
  if(!sysmode)
  {
    MemoryCopyUserToSystem (currentPCB, (char *)trapArgs, (char *)&arg, sizeof (arg));
  } 
  else 
  {
    bcopy ((void *)trapArgs, (void *)&arg, sizeof (arg));
  }

  return arg;
}
//--------------------------------------------------------------------
//
// int process_create(char *exec_name, ...);
//
// Here we support reading command-line arguments.  Maximum MAX_ARGS 
// command-line arguments are allowed.  Also the total length of the 
// arguments including the terminating '\0' should be less than or 
// equal to SIZE_ARG_BUFF.
//
//--------------------------------------------------------------------
static void TrapProcessCreateHandler(uint32 *trapArgs, int sysmode) {
  char allargs[SIZE_ARG_BUFF];  // Stores full string of arguments (unparsed)
  char name[PROCESS_MAX_NAME_LENGTH]; // Local copy of name of executable (100 chars or less)
  char *username=NULL;          // Pointer to user-space address of exec_name string
  int i=0, j=0;                 // Loop index variables
  char *args[MAX_ARGS];         // All parsed arguments (char *'s)
  int allargs_position = 0;     // Index into current "position" in allargs
  char *userarg = NULL;         // Current pointer to user argument string
  int numargs = 0;              // Number of arguments passed on command line

  dbprintf('p', "TrapProcessCreateHandler: function started\n");
  // Initialize allargs string to all NULL's for safety
  for(i=0;i<SIZE_ARG_BUFF; i++) {
    allargs[i] = '\0';
  }

  // First deal with user-space addresses
  if(!sysmode) {
    dbprintf('p', "TrapProcessCreateHandler: creating user process\n");
    // Get the known arguments into the kernel space.
    // Argument 0: user-space pointer to name of executable
    MemoryCopyUserToSystem (currentPCB, (char *)(trapArgs+0), (char *)&username, sizeof(char *));

    // Copy the user-space string at user-address "username" into kernel space
    for(i=0; i<PROCESS_MAX_NAME_LENGTH; i++) {
      MemoryCopyUserToSystem(currentPCB, (char *)(username+i), (char *)&(name[i]), sizeof(char));
      // Check for end of user-space string
      if (name[i] == '\0') break;
    }
    dbprintf('p', "TrapProcessCreateHandler: just parsed executable name (%s) from trapArgs\n", name);
    if (i == PROCESS_MAX_NAME_LENGTH) {
      printf("TrapProcessCreateHandler: length of executable filename longer than allowed!\n");
      exitsim();
    }

    // Copy the program name into "allargs", since it has to be the first argument (i.e. argv[0])
    allargs_position = 0;
    dstrcpy(&(allargs[allargs_position]), name);
    allargs_position += dstrlen(name) + 1; // The "+1" is so we're pointing just beyond the NULL

    // Rest of arguments: a series of char *'s until we hit NULL or MAX_ARGS
    for(i=0; i<MAX_ARGS; i++) {
      // First, must copy the char * itself into kernel space in order to read its value
      MemoryCopyUserToSystem(currentPCB, (char *)(trapArgs+1+i), (char *)&userarg, sizeof(char *));
      // If this is a NULL in the set of char *'s, this is the end of the list
      if (userarg == NULL) break;
      // Store a pointer to the kernel-space location where we're copying the string
      args[i] = &(allargs[allargs_position]);
      // Copy the string into the allargs, starting where we left off last time through this loop
      for (j=0; j<SIZE_ARG_BUFF; j++) {
        MemoryCopyUserToSystem(currentPCB, (char *)(userarg+j), (char *)&(allargs[allargs_position]), sizeof(char));
        // Move current character in allargs to next spot
        allargs_position++;
        // Check that total length of arguments is still ok
        if (allargs_position == SIZE_ARG_BUFF) {
          printf("TrapProcessCreateHandler: strlen(all arguments) > maximum length allowed!\n");
          exitsim();
        }
        // Check for end of user-space string
        if (allargs[allargs_position-1] == '\0') break;
      }
    }
    if (i == MAX_ARGS) {
      printf("TrapProcessCreateHandler: too many arguments on command line (did you forget to pass a NULL?)\n");
      exitsim();
    }
    numargs = i+1;
    // Arguments are now setup
  } else {
    // Addresses are already in kernel space, so just copy into our local variables 
    // for simplicity
    // Argument 0: (char *) name of program
    dstrncpy(name, (char *)(trapArgs[0]), PROCESS_MAX_NAME_LENGTH);
    // Copy the program name into "allargs", since it has to be the first argument (i.e. argv[0])
    allargs_position = 0;
    dstrcpy(&(allargs[allargs_position]), name);
    allargs_position += dstrlen(name) + 1;  // The "+1" is so that we are now pointing just beyond the "null" in the name
    allargs_position = 0;
    for (i=0; i<MAX_ARGS; i++) {
      userarg = (char *)(trapArgs[i+1]);
      if (userarg == NULL) break; // found last argument
      // Store the address of where we're copying the string
      args[i] = &(allargs[allargs_position]);
      // Copy string into allargs
      for(j=0; j<SIZE_ARG_BUFF; j++) {
        allargs[allargs_position] = userarg[j];
        allargs_position++;
        if (allargs_position == SIZE_ARG_BUFF) {
          printf("TrapProcessCreateHandler: strlen(all arguments) > maximum length allowed!\n");
          exitsim();
        }
        // Check for end of user-space string
        if (allargs[allargs_position-1] == '\0') break;
      }
    }
    if (i == MAX_ARGS) {
      printf("TrapProcessCreateHandler: too many arguments on command line (did you forget to pass a NULL?)\n");
      exitsim();
    }
    numargs = i+1;
  }

  ProcessFork(0, (uint32)allargs, name, 1);
}


//----------------------------------------------------------------------
//
//	TrapPrintfHandler
//
//	Handle a printf trap.here.
//	Note that the maximum format string length is PRINTF_MAX_FORMAT_LENGTH 
//      characters.  Exceeding this length could have unexpected results.
//
//----------------------------------------------------------------------
static void TrapPrintfHandler(uint32 *trapArgs, int sysMode) {
  char formatstr[PRINTF_MAX_FORMAT_LENGTH];  // Copy user format string here
  char *userformatstr=NULL;                  // Holds user-space address of format string
  int i,j;                                   // Loop index variables
  int numargs=0;                             // Keeps track of number of %'s (i.e. number of arguments)
  uint32 args[PRINTF_MAX_ARGS];              // Keeps track of all our args
  char *userstr;                             // Holds user-space address of the string that goes with any "%s"'s
  char strings_storage[PRINTF_MAX_STRING_ARGS][PRINTF_MAX_STRING_ARG_LENGTH]; // Places to copy strings for "%s" into
  int string_argnum=0;                       // Keeps track of how many "%s"'s we have received (index into strings_storage)

  if (!sysMode) {
    // Copy user-space format string into kernel space
    // Argument 0: format string
    MemoryCopyUserToSystem(currentPCB, (char *)(trapArgs+0), (char *)&userformatstr, sizeof(char *));
    // Copy format string itself from user space to kernel space
    for(i=0; i<PRINTF_MAX_FORMAT_LENGTH; i++) {
      MemoryCopyUserToSystem(currentPCB, (char *)(userformatstr + i), (char *)&(formatstr[i]), sizeof(char));
      // Check for end of string
      if (formatstr[i] == '\0') break;
    }
    if (i == PRINTF_MAX_FORMAT_LENGTH) { // format string too long
      printf("TrapPrintfHandler: format string too long!\n");
      return;
    }
  } else {
    // Not in system mode, can just copy string directly
    dstrncpy(formatstr, (char *)(trapArgs+0), PRINTF_MAX_FORMAT_LENGTH);
  }

  // Now read format string to find all the %'s
  numargs = 0;
  string_argnum = 0;
  for(i=0; i<dstrlen(formatstr); i++) {
    if (formatstr[i] == '%') {
      // Check for "%%"
      i++; // Skip over % symbol to look at the next character
      switch(formatstr[i]) {
        case '%': continue; // Skip over "%%"
          break;
        case 'c': // chars
                  // This argument is a single char value, but I think it's stored as a full int value.
                  // If it isn't, then I'm not sure how to call the *real* printf below with some non-4-byte arguments.
                  // The "+1" is because trapArgs[0] is the format string
                  if (!sysMode) {
                    MemoryCopyUserToSystem(currentPCB, (char *)(trapArgs+numargs+1), (char *)&(args[numargs]), sizeof(int));
                  } else {
                    args[numargs] = trapArgs[numargs+1];
                  }
                  numargs++;
          break;
        case 'l': if (formatstr[i+1] != 'f') {
                    printf("TrapPrintfHandler: unrecognized format character (l%c)\n", formatstr[i+1]);
                    return;
                  }
                  i++;
          // There is intentionally no "break" here so that the %lf and %f cases do the same thing.
          // This is what the previous Printf did, but I'm not sure yet that it is right.
        case 'f': // double floating points (64 bits)
        case 'g':
        case 'e': if (!sysMode) {
                    MemoryCopyUserToSystem(currentPCB, (char *)(trapArgs+numargs+1), (char *)&(args[numargs]), sizeof(double));
                  } else {
                    args[numargs] = trapArgs[numargs+1];
                    args[numargs+1] = trapArgs[numargs+2];
                  }
                  numargs += 2; // numargs is incremented by 2 since a double is the size of 2 ints
          break;
        case 'd': // integers
                  if (!sysMode) {
                    MemoryCopyUserToSystem(currentPCB, (char *)(trapArgs+numargs+1), (char *)&(args[numargs]), sizeof(int));
                  } else {
                    args[numargs] = trapArgs[numargs+1];
                  }
                  numargs++;
          break;
        case 's': // string
                  if (!sysMode) {
                    // First make sure we still have local string storage space available
                    if (string_argnum == PRINTF_MAX_STRING_ARGS) {
                      printf("TrapPrintfHandler: too many %%s arguments passed!\n");
                      return;
                    }
                    // Now copy the user-space address of the string into kernel space
                    MemoryCopyUserToSystem(currentPCB, (char *)(trapArgs+numargs+1), (char *)&userstr, sizeof(char *));
                    // Now copy each individual character from that string into kernel space
                    for(j=0; j<PRINTF_MAX_STRING_ARG_LENGTH; j++) {
                      MemoryCopyUserToSystem(currentPCB, (char *)(userstr+j), (char *)&(strings_storage[string_argnum][j]), sizeof(char));
                      // Check for end of user-space string
                      if (strings_storage[string_argnum][j] == '\0') break;
                    }
                    if (j == PRINTF_MAX_STRING_ARG_LENGTH) { // String was too long!
                      printf("TrapPrintfHandler: argument #%d (a string) was too long to print!\n", numargs);
                      return;
                    }
                    // Put the address of this kernel-space string as the argument for printf
                    args[numargs] = (int)(strings_storage[string_argnum]);
                    string_argnum++;
                  } else { // kernel space already, just copy address out of trapArgs
                    args[numargs] = trapArgs[numargs+1];
                  }
                  numargs++;
          break;
        default: printf("TrapPrintfHandler: unrecognized format character (%c)!\n", formatstr[i+1]);
                 return;
      }
    }
  }

  // Now we can print them all out
  switch(numargs) {
    case  0: printf(formatstr); break;
    case  1: printf(formatstr, args[0]); break;
    case  2: printf(formatstr, args[0], args[1]); break;
    case  3: printf(formatstr, args[0], args[1], args[2]); break;
    case  4: printf(formatstr, args[0], args[1], args[2], args[3]); break;
    case  5: printf(formatstr, args[0], args[1], args[2], args[3], args[4]); break;
    case  6: printf(formatstr, args[0], args[1], args[2], args[3], args[4], args[5]); break;
    case  7: printf(formatstr, args[0], args[1], args[2], args[3], args[4], args[5], args[6]); break;
    case  8: printf(formatstr, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]); break;
    case  9: printf(formatstr, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]); break;
    case 10: printf(formatstr, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]); break;
    case 11: printf(formatstr, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10]); break;
    case 12: printf(formatstr, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11]); break;
    default: printf("TrapPrintfHandler: too many arguments (%d) passed!\n", numargs);
  }

  // Done!
}





//----------------------------------------------------------------------
//
//	doInterrupt
//
//	Handle an interrupt or trap.
//
//----------------------------------------------------------------------
void
dointerrupt (unsigned int cause, unsigned int iar, unsigned int isr,
	     uint32 *trapArgs)
{
  int	result;
  int	i;
  uint32	args[4];
  int	intrs;
  uint32 handle;
  int ihandle;

  dbprintf ('t',"Interrupt cause=0x%x iar=0x%x isr=0x%x args=0x%08x.\n",
	    cause, iar, isr, (int)trapArgs);
  // If the TRAP_INSTR bit is set, this was from a trap instruction.
  // If the bit isn't set, this was a system interrupt.
  if (cause & TRAP_TRAP_INSTR) {
    cause &= ~TRAP_TRAP_INSTR;
    switch (cause) {
    case TRAP_CONTEXT_SWITCH:
      dbprintf ('t', "Got a context switch trap!\n");
      ProcessSchedule ();
      ClkResetProcess();
      break;
    case TRAP_EXIT:
    case TRAP_USER_EXIT:
      dbprintf ('t', "Got an exit trap!\n");
      ProcessDestroy (currentPCB);
      ProcessSchedule ();
      ClkResetProcess();
      break;
    case TRAP_PROCESS_FORK:
      dbprintf ('t', "Got a fork trap!\n");
      break;
    case TRAP_PROCESS_SLEEP:
      dbprintf ('t', "Got a process sleep trap!\n");
      ProcessSuspend (currentPCB);
      ProcessSchedule ();
      ClkResetProcess();
      break;
    case TRAP_PRINTF:
      // Call the trap printf handler and pass the arguments and a flag
      // indicating whether the trap was called from system mode.
      dbprintf ('t', "Got a printf trap!\n");
      TrapPrintfHandler (trapArgs, isr & DLX_STATUS_SYSMODE);
      break;
    case TRAP_OPEN:
      // Get the arguments to the trap handler.  If this is a user mode trap,
      // copy them from user space.
      if (isr & DLX_STATUS_SYSMODE) {
	args[0] = trapArgs[0];
	args[1] = trapArgs[1];
      } else {
	char	filename[32];
	// trapArgs points to the trap arguments in user space.  There are
	// two of them, so copy them to to system space.  The first argument
	// is a string, so it has to be copied to system space and the
	// argument replaced with a pointer to the string in system space.
	MemoryCopyUserToSystem (currentPCB, (char *)trapArgs, (char *)args, sizeof(args[0])*2);
	MemoryCopyUserToSystem (currentPCB, (char *)(args[0]), (char *)filename, 31);
	// Null-terminate the string in case it's longer than 31 characters.
	filename[31] = '\0';
	// Set the argument to be the filename
	args[0] = (uint32)filename;
      }
      // Allow Open() calls to be interruptible!
      intrs = EnableIntrs ();
      ProcessSetResult (currentPCB, args[1] + 0x10000);
      printf ("Got an open with parameters ('%s',0x%x)\n", (char *)(args[0]), args[1]);
      RestoreIntrs (intrs);
      break;
    case TRAP_CLOSE:
      // Allow Close() calls to be interruptible!
      intrs = EnableIntrs ();
      ProcessSetResult (currentPCB, -1);
      RestoreIntrs (intrs);
      break;
    case TRAP_READ:
      // Allow Read() calls to be interruptible!
      intrs = EnableIntrs ();
      ProcessSetResult (currentPCB, -1);
      RestoreIntrs (intrs);
      break;
    case TRAP_WRITE:
      // Allow Write() calls to be interruptible!
      intrs = EnableIntrs ();
      ProcessSetResult (currentPCB, -1);
      RestoreIntrs (intrs);
      break;
    case TRAP_DELETE:
      intrs = EnableIntrs ();
      ProcessSetResult (currentPCB, -1);
      RestoreIntrs (intrs);
      break;
    case TRAP_SEEK:
      intrs = EnableIntrs ();
      ProcessSetResult (currentPCB, -1);
      RestoreIntrs (intrs);
      break;
    case TRAP_PROCESS_GETPID:
      ProcessSetResult(currentPCB, GetCurrentPid()); 
      break;
    case TRAP_PROCESS_CREATE:
      TrapProcessCreateHandler(trapArgs, isr & DLX_STATUS_SYSMODE);
      break;
    case TRAP_SEM_CREATE:
      ihandle = GetIntFromTrapArg(trapArgs, isr & DLX_STATUS_SYSMODE);
      ihandle = SemCreate(ihandle);
      ProcessSetResult(currentPCB, ihandle); //Return handle
      break;
    case TRAP_SEM_WAIT:
      ihandle = GetIntFromTrapArg(trapArgs, isr & DLX_STATUS_SYSMODE);
      handle = SemHandleWait(ihandle);
      ProcessSetResult(currentPCB, handle); //Return 1 or 0
      break;
    case TRAP_SEM_SIGNAL:
      ihandle = GetIntFromTrapArg(trapArgs, isr & DLX_STATUS_SYSMODE);
      handle = SemHandleSignal(ihandle);
      ProcessSetResult(currentPCB, handle); //Return 1 or 0
      break;
    case TRAP_MALLOC:
      ihandle = GetIntFromTrapArg(trapArgs, isr & DLX_STATUS_SYSMODE);
      ihandle = (int)malloc(currentPCB, ihandle);
      ProcessSetResult(currentPCB, ihandle); //Return handle
      break;
    case TRAP_MFREE:
      ihandle = GetIntFromTrapArg(trapArgs, isr & DLX_STATUS_SYSMODE);
      ihandle = mfree(currentPCB, (void*)ihandle);
      ProcessSetResult(currentPCB, ihandle); //Return handle
      break;
    case TRAP_LOCK_CREATE:
      ihandle = LockCreate();
      ProcessSetResult(currentPCB, ihandle); //Return handle
      break;
    case TRAP_LOCK_ACQUIRE:
      ihandle = GetIntFromTrapArg(trapArgs, isr & DLX_STATUS_SYSMODE);
      handle = LockHandleAcquire(ihandle);
      ProcessSetResult(currentPCB, handle); //Return 1 or 0
      break;
    case TRAP_LOCK_RELEASE:
      ihandle = GetIntFromTrapArg(trapArgs, isr & DLX_STATUS_SYSMODE);
      handle = LockHandleRelease(ihandle);
      ProcessSetResult(currentPCB, handle); //Return 1 or 0
      break;
    case TRAP_COND_CREATE:
      ihandle = GetIntFromTrapArg(trapArgs, isr & DLX_STATUS_SYSMODE);
      ihandle = CondCreate(ihandle);
      ProcessSetResult(currentPCB, ihandle); //Return handle
      break;
    case TRAP_COND_WAIT:
      ihandle = GetIntFromTrapArg(trapArgs, isr & DLX_STATUS_SYSMODE);
      ihandle = CondHandleWait(ihandle);
      ProcessSetResult(currentPCB, ihandle); //Return 1 or 0
      break;
    case TRAP_COND_SIGNAL:
      ihandle = GetIntFromTrapArg(trapArgs, isr & DLX_STATUS_SYSMODE);
      ihandle = CondHandleSignal(ihandle);
      ProcessSetResult(currentPCB, ihandle); //Return 1 or 0
      break;
    case TRAP_COND_BROADCAST:
      ihandle = GetIntFromTrapArg(trapArgs, isr & DLX_STATUS_SYSMODE);
      ihandle = CondHandleBroadcast(ihandle);
      ProcessSetResult(currentPCB, ihandle); //Return 1 or 0
      break;
    default:
      printf ("Got an unrecognized trap (0x%x) - exiting!\n",
	      cause);
      exitsim ();
      break;
    }
  } else {
    switch (cause) {
    case TRAP_TIMER:
      dbprintf ('t', "Got a timer interrupt!\n");
      // ClkInterrupt returns 1 when 1 "process quantum" has passed, meaning
      // that it's time to call ProcessSchedule again.
      if (ClkInterrupt()) {
        ProcessSchedule ();
      }
      break;
    case TRAP_KBD:
      do {
	i = *((uint32 *)DLX_KBD_NCHARSIN);
	result = *((uint32 *)DLX_KBD_GETCHAR);
	printf ("Got a keyboard interrupt (char=0x%x(%c), nleft=%d)!\n",
		result, result, i);
      } while (i > 1);
      break;
    case TRAP_ACCESS:
      printf ("Exiting after illegal access at iar=0x%x, isr=0x%x\n", iar, isr);
      exitsim ();
      break;
    case TRAP_ADDRESS:
      printf ("Exiting after illegal address at iar=0x%x, isr=0x%x\n",
	      iar, isr);
      exitsim ();
      break;
    case TRAP_ILLEGALINST:
      printf ("Exiting after illegal instruction at iar=0x%x, isr=0x%x\n",
	      iar, isr);
      exitsim ();
      break;
    case TRAP_PAGEFAULT:
      MemoryPageFaultHandler(currentPCB);
      break;
    default:
      printf ("Got an unrecognized system interrupt (0x%x) - exiting!\n",
	      cause);
      exitsim ();
      break;
    }
  }
  dbprintf ('t',"About to return from dointerrupt.\n");
  // Note that this return may schedule a new process!
  intrreturn ();
}

