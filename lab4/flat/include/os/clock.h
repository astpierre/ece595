#ifndef __CLOCK_DLX__
#define __CLOCK_DLX__

// Define default clock resolution (i.e. the timer interrupt will go off every
// so many milliseconds.
// Note: do not decrease the default resolution lower than 1 ms (1000 us), otherwise
// ProcessSchedule doesn't work for some reason.
#define CLOCK_DEFAULT_RESOLUTION 1000 // in microseconds
// Define the default number of jiffies which must pass before ProcessSchedule
// is called.
#define CLOCK_PROCESS_JIFFIES    (10000/CLOCK_DEFAULT_RESOLUTION) // Call Process Schedule 

void ClkModuleInit();   // Initializes the clock module
void ClkStart();        // Starts the clock firing
void ClkStop();         // Stops the clock
int ClkInterrupt();     // Called from traps.c when a timer interrupt occurs
inline void ClkSetResolution(int usec); // Sets resolution of clock, in microseconds
inline int ClkGetResolution(); // Returns the resolution of the clock, in microseconds
inline double ClkGetCurTime();    // Returns number of milliseconds since clock was started
inline int ClkGetCurJiffies(); // Returns number of jiffies that have fired since clock started
void ClkResetProcess();  // Resets the current process counter to the current time

#endif
