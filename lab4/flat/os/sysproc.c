//
//	sysproc.c
//
//	This file defines system processes run by the OS.  It also
//	includes a routine to initialize system processes.

/*
//----------------------------------------------------------------------
//
//	doSomething
//
//	A useless little function that does some small stuff to test
//	process switching and the like.
//
//----------------------------------------------------------------------
static Sem	mysems[10];
static
void
doSomething (int me)
{
  int		i;
  int		j;
  int		mygrp = me % 10;
  char	buf[40];
  char	buf2[40];

  if (me < 100) {
    SemInit (&(mysems[mygrp]), 1);
    for (i = 1; i <= 6; i++) {
      dstrcpy (buf, "Process #");
      ditoa (i, buf2);
      dstrcat (buf, buf2);
      ProcessFork (&doSomething, i * me * 1000 + me, 0, 0, buf, 0);
    }
  }
  SemWait (&mysems[mygrp]);
  for (i = 0, j = 0; i < (me * 2); i++) {
    if ((i % 1000) == 0) {
      printf ("Running process %d (iteration %d)!\n", me, i);
    }
    j += (i & 3);
  }
  SemSignal (&mysems[mygrp]);
}

//----------------------------------------------------------------------
//
//	chaseTail
//
//	A little function that chases its tail, forking off other
//	processes before exiting.  This can be used to test resource
//	allocation and reclamation.
//
//----------------------------------------------------------------------
static
void
chaseTail (int me)
{
  char	*str = "chasetail";

  printf ("Chasing tail %d.\n", me);
  printf ("Open of (%s,0x%x) returns 0x%x.\n", str, me+0x2000,
	  Open (str, me + 0x2000));
  if (me < 40) {
    ProcessFork (&chaseTail, me + 1, 0, 0, "ChaseTail", 0);
  }
}

//----------------------------------------------------------------------
//
//	emptyLoop
//
//	This is an empty loop.  It's used to test running code.  A similar
//	loop could be used as the "idle" process.
//
//----------------------------------------------------------------------
static
void
emptyLoop (int me)
{
  int	i;
  int	limit = me * 50000;

  i = 0;
  while (1) {
    i += 1;
    if (i > limit) {
      i = 0;
      printf ("%d ", me);
    }
  }
}

//----------------------------------------------------------------------
//
//	randomStuff
//
//	This routine tests random number usage in DLXOS.
//
//----------------------------------------------------------------------
static
void
randomStuff (int seed)
{
  int		i,rn;
  extern int	random(),srandom();

  srandom (seed);

  for (i = 0; i < 40; i++) {
    rn = random ();
    printf ("Random number %02i is 0x%08x.\n", i, rn);
  }
}

//----------------------------------------------------------------------
//
//	SysprocCreateProcesses
//
//	This is the routine that is called by main to create processes.
//	It must create at least one process, and there must always be
//	at least one runnable process.  If there isn't, the OS will
//	exit.
//
//----------------------------------------------------------------------
void
SysprocCreateProcesses ()
{
  int		i;

  for (i = 0; i < 10; i++) {
    SemInit (&mysems[i], 1);
  }

//  ProcessFork (&emptyLoop, 1, "Loop1", 0);
//  ProcessFork (&emptyLoop, 2, "Loop1", 0);
//  ProcessFork (&emptyLoop, 3, "Loop1", 0);
//  ProcessFork (&doSomething, 1, "Creator1", 0);
//  ProcessFork (&doSomething, 2, "Creator2", 0);
//  ProcessFork (&randomStuff, 1234, "randomStuff", 0);
//  ProcessFork (&chaseTail, 10, "ChaseTail", 0);
}
*/
