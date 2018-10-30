//--------------------------------------------------------------
// Contains functions for maintaining a global clock time
//--------------------------------------------------------------

#include "dlxos.h"
#include "traps.h"
#include "ostraps.h"
#include "process.h"
#include "clock.h"

// Definition: a "jiffy" is defined as the minimum clock resolution.  For instance,
// if timer interrupts are set to go off every 1 millisecond, then 1 jiffy = 1 milliseond.

static int curtime = 0;              // The number of "jiffies" that have passed
static int clock_resolution = CLOCK_DEFAULT_RESOLUTION;   // Number of microseconds in one "jiffy"
static int clock_running = 0;        // Flag to enable starting/stopping clock
static int last_trigger_jiffies = 0; // Keeps track of last time we triggered ProcessSchedule

//-------------------------------------------------------------
//
// ClkModuleInit sets the default clock resolution and sets up
// other global variables as necessary to prepare for a call
// to ClkStart()
//
//-------------------------------------------------------------
void ClkModuleInit() {
  curtime = 0;
  clock_resolution = CLOCK_DEFAULT_RESOLUTION; // 100 usec per jiffy
  clock_running = 0;
}

//-------------------------------------------------------------
// ClkStart starts timer interrupts 
//-------------------------------------------------------------
void ClkStart() {
  clock_running = 1;
  *((int *)DLX_TIMER_ADDRESS) = clock_resolution; // in microseconds
  dbprintf('c', "ClkStart: clock started\n");
}

//-------------------------------------------------------------
// ClkStop stops timer interrupts from occurring.
//-------------------------------------------------------------
void ClkStop() {
  clock_running = 0;
}

//-------------------------------------------------------------
// ClkInterrupt is called in traps.c when a timer interrupt
// occurs.  Returns 1 if enough jiffies have passed to 
// trigger a call to ProcessSchedule.  Returns 0 otherwise.
//-------------------------------------------------------------
int ClkInterrupt() {
  if (clock_running) {
    curtime++;
    // Set the timer to go off again so that it can be
    // ticking away while we're in ProcessSchedule.
    *((int *)DLX_TIMER_ADDRESS) = clock_resolution;

    // Now check to see if enough jiffies have occurred to trigger
    // another ProcessSchedule
    if (curtime - last_trigger_jiffies > CLOCK_PROCESS_JIFFIES) {
      last_trigger_jiffies = curtime;
      dbprintf('c', "ClkInterrupt: calling ProcessSchedule\n");
      return 1; // can call ProcessSchedule
    }
    return 0; // too soon to call ProcessSchedule
  }
  return 0; // Clock isn't running, so don't call ProcessSchedule
}

//-------------------------------------------------------------
// ClkSetResolution changes the resolution of the clock,
// in microseconds per jiffy.
//-------------------------------------------------------------
inline void ClkSetResolution(int new_res) {
  clock_resolution = new_res;
}

//-------------------------------------------------------------
// ClkGetResolution returns the current clock resolution in
// microseconds per jiffy.
//-------------------------------------------------------------
inline int ClkGetResolution() {
  return clock_resolution;
}

//-------------------------------------------------------------
// ClkGetCurTime returns the number of seconds (and fractional 
// seconds) since the clock started.  Note that this is 
// inaccurate if the clock resolution was ever changed after 
// calling ClkStart.
//-------------------------------------------------------------
inline double ClkGetCurTime() {
  // First convert clock_resolution to seconds (divide by 1 million),
  // then multiply by number of jiffies.
  return (double)clock_resolution / (double)1000000 * (double)curtime;
}

//-------------------------------------------------------------
// ClkGetCurJiffies returns the number of jiffies that have
// passed since the clock started.
//-------------------------------------------------------------
inline int ClkGetCurJiffies() {
  return curtime;
}

//-------------------------------------------------------------
// ClkResetProcess resets the process jiffies counter
//-------------------------------------------------------------
void ClkResetProcess() {
  last_trigger_jiffies = curtime;
}
