//
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "memory.h"
#include "process.h"
#include "queue.h"

static uint32	pagestart;
static int	freemapmax;
static int	nfreepages;
static uint32	freepages[MEMORY_MAX_PAGES/32];
static uint32	negativeone = 0xffffffff;

//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------
static
inline
uint32
invert (uint32 n)
{
  return (n ^ negativeone);
}

//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int
MemoryGetSize ()
{
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}

//----------------------------------------------------------------------
//
//	MemorySetFreemap
//
//----------------------------------------------------------------------
inline
void
MemorySetFreemap (int p, int b)
{
  uint32	wd = p / 32;
  uint32	bitnum = p % 32;

  freepages[wd] = (freepages[wd] & invert(1 << bitnum)) | (b << bitnum);
  dbprintf ('m', "Set freemap entry %d to 0x%x.\n",
	    wd, freepages[wd]);
}


//----------------------------------------------------------------------
//
//	MemoryInitModule
//
//	Initialize the memory module of the operating system.
//
//----------------------------------------------------------------------
void
MemoryModuleInit ()
{
  int		i;
  int		maxpage = MemoryGetSize () / MEMORY_PAGE_SIZE;
  int		curpage;

  pagestart = (lastosaddress + MEMORY_PAGE_SIZE - 4) / MEMORY_PAGE_SIZE;
  freemapmax = (maxpage+31) / 32;
  dbprintf ('m', "Map has %d entries, memory size is 0x%x.\n",
	    freemapmax, maxpage);
  dbprintf ('m', "Free pages start with page # 0x%x.\n", pagestart);
  for (i = 0; i < freemapmax; i++) {
    // Initially, all pages are considered in use.  This is done to make
    // sure we don't have any partially initialized freemap entries.
    freepages[i] = 0;
  }
  nfreepages = 0;
  for (curpage = pagestart; curpage < maxpage; curpage++) {
    nfreepages += 1;
    MemorySetFreemap (curpage, 1);
  }
  dbprintf ('m', "Initialized %d free pages.\n", nfreepages);
}

//----------------------------------------------------------------------
//
//	MemoryAllocPage
//
//	Allocate a page of memory.
//
//----------------------------------------------------------------------
inline
int
MemoryAllocPage ()
{
  static int	mapnum = 0;
  int		bitnum;
  uint32	v;

  if (nfreepages == 0) {
    return (0);
  }
  dbprintf ('m', "Allocating memory, starting with page %d\n", mapnum);
  while (freepages[mapnum] == 0) {
    mapnum += 1;
    if (mapnum >= freemapmax) {
      mapnum = 0;
    }
  }
  v = freepages[mapnum];
  for (bitnum = 0; (v & (1 << bitnum)) == 0; bitnum++) {
  }
  freepages[mapnum] &= invert(1 << bitnum);
  v = (mapnum * 32) + bitnum;
  dbprintf ('m', "Allocated memory, from map %d, page %d, map=0x%x.\n",
	    mapnum, v, freepages[mapnum]);
  nfreepages -= 1;
  return (v);
}

//----------------------------------------------------------------------
//
//	MemoryFreePage
//
//	Free a page of memory.
//
//----------------------------------------------------------------------
void
MemoryFreePage(uint32 page)
{
  MemorySetFreemap (page, 1);
  nfreepages += 1;
  dbprintf ('m',"Freed page 0x%x, %d remaining.\n", page, nfreepages);
}

//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  This works for simple one-level
//	page tables, but will have to be modified for two-level page
//	tables.
//
//----------------------------------------------------------------------
uint32
MemoryTranslateUserToSystem (PCB *pcb, uint32 addr)
{
    int	page = addr / MEMORY_PAGE_SIZE;
    int offset = addr % MEMORY_PAGE_SIZE;

    if (page > pcb->npages) {
      return (0);
    }
    return ((pcb->pagetable[page] & MEMORY_PTE_MASK) + offset);
}

//----------------------------------------------------------------------
//
//	moveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
static
int
moveBetweenSpaces (PCB *pcb, unsigned char *s, unsigned char *u,
			 int n, int dir)
{
  unsigned char *curUser;
  int		bytesCopied = 0;
  int		bytesToCopy;

  while (n > 0) {
    // Translate current user page to system address.  If this fails, return
    // the number of bytes copied so far.
    curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)u);
    if (curUser == (unsigned char *)0) {
      // Leave the loop if translation fails.
      break;
    }
    // Calculate the number of bytes to copy this time.  This is the minimum
    // of the bytes left on this page and the bytes left to copy.
    bytesToCopy = MEMORY_PAGE_SIZE - ((uint32)curUser % MEMORY_PAGE_SIZE);
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }
    // Perform the copy.
    if (dir >= 0) {
      bcopy (s, curUser, bytesToCopy);
    } else {
      bcopy (curUser, s, bytesToCopy);
    }
    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;
    bytesCopied += bytesToCopy;
    s += bytesToCopy;
    u += bytesToCopy;
  }
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int
MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n)
{
  return (moveBetweenSpaces (pcb, from, to, n, 1));
}

int
MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n)
{
  return (moveBetweenSpaces (pcb, to, from, n, -1));
}


//----------------------------------------------------------------------
//
// MemorySetupPte
//
//	Set up a PTE given a page number.
//
//----------------------------------------------------------------------
uint32
MemorySetupPte (uint32 page)
{
  return ((page * MEMORY_PAGE_SIZE) | MEMORY_PTE_VALID);
}

//----------------------------------------------------------------------
//
// MemoryFreePte
//
//	Free a page given its PTE.
//
//----------------------------------------------------------------------
void
MemoryFreePte (uint32 pte)
{
  MemoryFreePage ((pte & MEMORY_PTE_MASK) / MEMORY_PAGE_SIZE);
}

//----------------------------------------------------------------------
//
//	MemoryPteToPage
//
//	Given a PTE, return the base address for the (virtual) page the
//	PTE references.
//
//----------------------------------------------------------------------
uint32
MemoryPteToPage (uint32 pte)
{
  return (pte & MEMORY_PTE_MASK);
}

//----------------------------------------------------------------------
//
//	MemoryGetOperandAddress
//
//	This routine takes an instruction and a pointer to a PCB, and
//	returns the address that the instruction is trying to use
//	for its memory operand.  If the instruction doesn't access memory,
//	it returns 0xffffffff.  Note that this means it's impossible to
//	distinguish between a valid access to 0xffffffff and an instruction
//	without an operand....
//
//----------------------------------------------------------------------
uint32
MemoryGetOperandAddress (PCB *pcb, uint32 instr)
{
  static uint32	invalidOps = 0x3404;
  uint32	opcode = (instr >> 26) & 0x3f;
  uint32	reg;
  uint32	addr;
  uint32	offset;
  uint32	regValue;

  if ((opcode<0x20) || (opcode>0x2f) || ((invalidOps >> (opcode-0x20) & 1))) {
    return (0xffffffff);
  }
  reg = (instr >> 21) & 0x1f;
  offset = instr & 0xffff;
  if (offset & 0x8000) {
    offset |= 0xffff0000;
  }
  regValue = pcb->currentSavedFrame[PROCESS_STACK_IREG+reg];
  addr = regValue + offset;
  dbprintf ('m', "Operand for instruction %08x is 0x%x (reg=<%d,0x%x>)\n",
	    instr, addr, reg, regValue);
  return (addr);
}
