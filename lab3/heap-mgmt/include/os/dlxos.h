//
//  Routines used by the entire operating system.
//

#ifndef  __dlxos_h__
#define  __dlxos_h__

#include "misc.h"

typedef  unsigned int  uint32;

#define  DLX_PROCESS_QUANTUM  10000    // in microseconds

extern void  SetTimer (int);

extern char  debugstr[];

#define  ASSERT(cond,s)  (cond ? 0 : printf ("%s: %s\n", __FUNCTION__,s))

// dbprintf() is a VERY useful macro.  It gets used as follows:
// dbprintf ('x', "This prints %d and %x\n", val1, val2);
// This will print the expected string only if the debugging options contain
// the letter 'x'.  This allows users to turn on different debugging
// statements at different times by using different letters.  For example,
// process debugging statements could use 'p', and memory 'm'.  Specifying
// a '+' in the debugging string will turn on all debugging printfs.
/*#define  dbprintf(flag, format, args...)     \
    if ((dindex(debugstr,flag)!=(char *)0) ||  \
  (dindex(debugstr,'+')!=(char *)0)) {  \
  printf (format, ## args);    \
    } */
#define  dbprintf(flag, format, args...) \
         if (dindex(debugstr,flag)!=(char *)0) { printf(format, ## args); }  \
         if (dindex(debugstr,'+')!=(char *)0)  { printf(format, ## args); }


extern int  CurrentIntrs ();
extern int  SetIntrs (int);
extern void  KbdModuleInit ();
extern void  intrreturn ();

inline int
DisableIntrs ()
{
  return (SetIntrs (0xf));
}

inline int
EnableIntrs ()
{
  return (SetIntrs (0x0));
}

inline int
RestoreIntrs (int intrs)
{
  return (SetIntrs (intrs));
}


#endif  // _dlxos_h_

