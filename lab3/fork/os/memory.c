//
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";
#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "queue.h"

// Freemap is an array of 32-bit integers
//  Each bit represents a page, so we create the free
//  map to be composed of MEM_MAX_PAGES/32 ints.
static uint32 freemap[MEM_MAX_PAGES >> 5];
static char pagereferencecounter[MEM_MAX_SIZE >> MEM_L1FIELD_FIRST_BITNUM];
static uint32 pagestart;
static int nfreepages;
static int freemapmax;
static int physicalpgmax; 
//----------------------------------------------------------------------
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
static inline uint32 invert (uint32 n) 
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
int MemoryGetSize() 
{
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}

//---------------------------------------------------------------------
//  MemoryAllocPage 
//---------------------------------------------------------------------      
int MemoryAllocPage(void) 
{
    uint32 physical_page = (lastosaddress>>MEM_L1FIELD_FIRST_BITNUM);
    uint32 idx = physical_page >> 5;
    uint32 page_number = 0;
    
    // Debug print statement
    dbprintf('m', "MemoryAllocPage: (PID:%d) function started\n",GetCurrentPid());
    
    // First cycle through the integer freemap vales, break at first !inuse
    while(idx<(MEM_MAX_PAGES>>5) && (freemap[idx] == MEM_FREEMAP_INUSE)) idx++;

    // Check if we over indexed
    if(idx == MEM_MAX_PAGES >> 5) return MEM_FAIL;

    // Scroll through bits in integer to find free page
    while(freemap[idx] << page_number++);

    // The physical page is the index in the freemap * 32 + page_number - 1
    physical_page = (idx<<5) + page_number - 1;
    pagereferencecounter[physical_page] = 1;
    nfreepages -= 1;

    // Mark in freemap that page now in use
    freemap[idx] |= (uint32)(0x1 << (32-page_number));

    // Check that physical page does not exceed memory size
    if(physical_page > ((MemoryGetSize()-1) >> MEM_L1FIELD_FIRST_BITNUM)) return MEM_FAIL;

    // Else, return the physical page number
    return physical_page;
}

//---------------------------------------------------------------------
//  MemorySetupPTE ~ setup a page table entry given phys page number
//---------------------------------------------------------------------      
uint32 MemorySetupPTE (uint32 page)
{  return ((page<<MEM_L1FIELD_FIRST_BITNUM) | MEM_PTE_VALID);  }

//---------------------------------------------------------------------
//  MemoryFreePTE ~ free a page given a PTE
//---------------------------------------------------------------------      
void MemoryFreePTE (uint32 pte)
{  MemoryFreePage(pte & MEM_PTE_TO_PAGEADDRESS_MASK);  }
    
//---------------------------------------------------------------------
//  MemorySharePage ~ share a page given its PTE
//---------------------------------------------------------------------      
void MemorySharePage (uint32 pte)
{
    int p = ((pte & MEM_PTE_MASK) / MEM_PAGESIZE);
    pagereferencecounter[p] += 1;
    return;
}

//---------------------------------------------------------------------
//  MemoryFreePage ~ free a page given its PTE
//---------------------------------------------------------------------      
void MemoryFreePage(uint32 page)
{
    // Find the offset of the physical page from 32-bit intervals
    // Use the offset to change the value of corresponding page
    // in the freemap
    uint32 offset = (uint32)(page & MEM_FREEMAP_PAGEOFFSET_MASK);
    
    pagereferencecounter[page] -= 1;
    if(pagereferencecounter[page] > 0) return;

    // Debug print statement
    dbprintf('m', "MemoryFreePage: (PID:%d) function started\n",GetCurrentPid());
    
    // Use the page to find index in freemap of target int
    freemap[page>>5] &= invert(0x1 << (31-offset));
    nfreepages += 1;
}

//----------------------------------------------------------------------
//
//	MemoryModuleInit
//
//	Initialize the memory module of the operating system.
//  Basically just need to setup the freemap for pages, and mark
//  the ones in use by the operating system as "VALID", and mark
//  all the rest as not in use.
//
//----------------------------------------------------------------------
void MemoryModuleInit() 
{
    //-----------------------------------------------------
    // Freemap: 
    //          An array of bits in memory representing
    //          which pages are in use and which are not
    // 
    // Implementation:  
    //          Array of integers, each integer represents
    //          32 pages. To find an availible physical
    //          page, loop through the freemap ints until
    //          you find one entry that is not zero. Then
    //          start looking at each bit one-at-a-time
    //          until you find a bit that is a one.
    // Init:
    //          Mark the pages in use by OS as "INUSE" and
    //          the rest as "NOTINUSE"... But how can I 
    //          know which pages those are??? Well, the OS
    //          is loaded at the top of physical memory.
    //          So, it must be starting at address 0x0,
    //          what about ending? DLXOS defines a global
    //          var in /os/osend.s at compile time called:
    //          lastosaddress.  
    //-----------------------------------------------------
    int idx=0;
    uint32 lastosaddr_page = (lastosaddress>>MEM_L1FIELD_FIRST_BITNUM);
    uint32 page_freemapoffset = (lastosaddr_page & MEM_FREEMAP_PAGEOFFSET_MASK);

    physicalpgmax = MemoryGetSize() / MEM_PAGESIZE;
    pagestart = (lastosaddress + MEM_PAGESIZE - 4) / MEM_PAGESIZE;
    freemapmax = (physicalpgmax + 31) / 32;

    // Debug print statement
    dbprintf('m', "MemoryModuleInit: (PID:%d) function started\n",GetCurrentPid());

    // Set all pages to be in use initially
    for(idx=0; idx<freemapmax; idx++) freemap[idx] = MEM_FREEMAP_INUSE;

    // Set references to zero for all pages
    for(idx=0; idx<physicalpgmax; idx++) pagereferencecounter[idx] = 0;

    nfreepages=0;
    // Mark all OS pages as INUSE
    for(idx=0; idx<(lastosaddr_page>>5); idx++) 
    {  pagereferencecounter[idx] = 1;  }

    // Handle the last 32 pages used by OS (may not be using all 32)
    freemap[lastosaddr_page>>5] = (uint32)(MEM_FREEMAP_INUSE<<(31-page_freemapoffset));
    
    // Mark the rest of the pages as NOTINUSE
    for(idx=(lastosaddr_page>>5)+1; idx<MEM_MAX_PAGES>>5; idx++)
    {  freemap[idx]=MEM_FREEMAP_NOTINUSE; nfreepages++;  }
}

//----------------------------------------------------------------------
//  MemoryTranslateUserToSystem
//	  Translate a user address (in the process referenced by pcb)
//	  into an OS (physical) address.  Return the physical address.
//----------------------------------------------------------------------
uint32 MemoryTranslateUserToSystem(PCB *pcb, uint32 addr)
{
    uint32 physical_addr=0;
    int page_offset = addr & MEM_PAGE_OFFSET_MASK;
    int page_virtual = addr >> MEM_L1FIELD_FIRST_BITNUM;
    int pte = pcb->pagetable[page_virtual];

    // Debug print statement
    //dbprintf('m', "MemoryTranslateUserToSystem: (PID:%d) function started\n",GetCurrentPid());

    // If PTE invalid, page fault
    if((pte & MEM_PTE_VALID) == 0) return MemoryPageFaultHandler(pcb);

    // Else calculate physical address and return it
    physical_addr = (uint32)((pte&(~MEM_PAGE_OFFSET_MASK))+page_offset);
    return physical_addr;
}

//----------------------------------------------------------------------
//	MemoryMoveBetweenSpaces
//	  Copy data between user and system spaces
//	  This is done page by page by:
//	    * Translating the user address into system space
//	    * Copying all of the data in that page
//	    * Repeating until all of the data is copied
//	  
//	  A positive direction means: copy system --> user
//	  A negative direction means: copy user --> system
//	  
//	  This routine returns the number of bytes copied. Note that this
//	  may be less than the number requested if there were unmapped pages
//	  in the user range. If this happens, the copy stops at the
//	  first unmapped address.
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) 
{
    unsigned char *curUser; // Holds current physical address representing user-space virtual address
    int bytesCopied = 0;    // Running counter
    int	bytesToCopy;        // Used to compute number of bytes left in page to be copied
    
    // Debug print statement
    //dbprintf('m', "MemoryMoveBetweenSpaces: (PID:%d) function started\n",GetCurrentPid());
    
    while (n > 0)
    {
        // Translate current user page to system address
        // If this fails, return the number of bytes copied so far
        curUser = (unsigned char *)MemoryTranslateUserToSystem(pcb, (uint32)user);
        
        // If we could not translate address, exit now
        if (curUser == (unsigned char *)0) break;

        // Calculate the number of bytes to copy this time. If we have more bytes
        // to copy than there are left in the current page, we'll have to just copy to the
        // end of the page and then go through the loop again with the next page.
        
        // In other words, "bytesToCopy" is the minimum of the bytes left on this page 
        // and the total number of bytes left to copy ("n")

        // First, compute number of bytes left in this page. This is just
        // the total size of a page minus the current offset part of the physical
        // address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
        // MEM_PAGE_OFFSET_MASK should be the bit mask required to get just the
        // "offset" portion of an address.
        bytesToCopy = MEM_PAGESIZE - ((uint32)curUser & MEM_PAGE_OFFSET_MASK);
    
        // Now find minimum of bytes in this page vs. total bytes left to copy
        if (bytesToCopy > n) 
        {  bytesToCopy = n;  }

        // Perform positive direction copy
        if (dir >= 0) {  bcopy(system, curUser, bytesToCopy);  }
        // Perform negative direction copy
        else {  bcopy (curUser, system, bytesToCopy);  }

        // Keep track of bytes copied and adjust addresses appropriately.
        n -= bytesToCopy;           // Total number of bytes left to copy
        bytesCopied += bytesToCopy; // Total number of bytes copied thus far
        system += bytesToCopy;      // Current address in system space to copy next bytes from/into
        user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
  }
  return (bytesCopied);
}

//----------------------------------------------------------------------
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) 
{  return (MemoryMoveBetweenSpaces(pcb, from, to, n, 1));  }
int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) 
{  return (MemoryMoveBetweenSpaces(pcb, to, from, n, -1));  }

//---------------------------------------------------------------------
//  MemoryPageFaultHandler 
//      Called in traps.c whenever a page fault, or sementation fault
//      (better known as a "seg fault") occurs.  If the address that was
//      being accessed is on the stack, we need to allocate a new page 
//      for the stack.  If it is not on the stack, then this is a legitimate
//      seg fault and we should kill the process.  Returns MEM_SUCCESS
//      on success, and kills the current process on failure.  Note that
//      fault_address is the beginning of the page of the virtual address that 
//      caused the page fault, i.e. it is the vaddr with the offset zero-ed
//      out.
//---------------------------------------------------------------------
int MemoryPageFaultHandler(PCB *pcb) 
{
    // Calculate the address and virt+stack page numbers
    uint32 addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT];
    uint32 virtual_page_num = addr>>MEM_L1FIELD_FIRST_BITNUM;
    uint32 physical_page_num;
    uint32 stack_page_num = pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]>>MEM_L1FIELD_FIRST_BITNUM;

    // Debug print statement
    dbprintf('m', "MemoryPageFaultHandler: (PID:%d) function started\n",GetCurrentPid());

    // If user stack triggered Page fault, allocate page
    //  => return MEM_SUCCESS
    if(virtual_page_num >= stack_page_num)
    {
        physical_page_num = MemoryAllocPage();
        pcb->pagetable[virtual_page_num] = MemorySetupPTE(physical_page_num);
        return MEM_SUCCESS;
    }
    else
    {
        // Else, ProcessKill => return MEM_FAIL
        ProcessKill();
        return MEM_FAIL;
    }
}

void MemoryROPAccessHandler(PCB * pcb)
{
    uint32 faultAddress = pcb->currentSavedFrame[PROCESS_STACK_FAULT];
    uint32 virtual_page_num = faultAddress >> MEM_L1FIELD_FIRST_BITNUM;
    uint32 physical_page_num = ((pcb->pagetable[virtual_page_num]&MEM_PTE_MASK)>>MEM_L1FIELD_FIRST_BITNUM);
    uint32 newPage;

    if(pagereferencecounter[physical_page_num] > 1)
    {
        newPage = MemoryAllocPage();
        pcb->pagetable[virtual_page_num] = MemorySetupPTE(newPage);
        bcopy((char *)(faultAddress), (char *)(newPage * MEM_PAGESIZE), MEM_PAGESIZE);
        pagereferencecounter[physical_page_num] -= 1;
    }
    else pcb->pagetable[virtual_page_num] &= invert(MEM_PTE_READONLY);
}
