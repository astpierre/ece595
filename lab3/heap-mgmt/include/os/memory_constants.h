#ifndef	_memory_constants_h_
#define	_memory_constants_h_

//------------------------------------------------
// #define's that you are given:
//------------------------------------------------
// We can read this address in I/O space to figure out how much memory
// is available on the system.
#define	DLX_MEMSIZE_ADDRESS	0xffff0000

// Return values for success and failure of functions
#define MEM_SUCCESS 1
#define MEM_FAIL -1

//---------------------------------------------------------

#define MEM_L1FIELD_FIRST_BITNUM 12
// Use a virtual memory size of 1024KB
#define MEM_MAX_VIRTUAL_ADDRESS (1<<20)-1
// Use a max physical memory size of 2MB
#define MEM_MAX_SIZE (1<<21)
#define MEM_MAX_PAGES (MEM_MAX_SIZE>>MEM_L1FIELD_FIRST_BITNUM)

#define MEM_PTE_READONLY 0x4
#define MEM_PTE_DIRTY 0x2
#define MEM_PTE_VALID 0x1

#define MEM_PAGESIZE (0x1<<MEM_L1FIELD_FIRST_BITNUM)
#define MEM_MAX_PAGE_OFFSET (MEM_PAGESIZE-1)
#define MEM_PAGE_OFFSET_MASK (MEM_PAGESIZE-1)

#define MEM_PAGETABLE_SIZE ((MEM_MAX_VIRTUAL_ADDRESS + 1)>>MEM_L1FIELD_FIRST_BITNUM)
#define MEM_MAX_PAGETABLE_INDEX (MEM_PAGETABLE_SIZE-1)

#define MEM_PTE_TO_PAGEADDRESS_MASK ~(MEM_PTE_READONLY | MEM_PTE_DIRTY | MEM_PTE_VALID)

#define MEM_FREEMAP_PAGEOFFSET_MASK (uint32)0x1F
#define MEM_FREEMAP_INUSE 0xFFFFFFFF
#define MEM_FREEMAP_NOTINUSE 0x00000000

#define MEM_MAX_HEAP_NODES 128

typedef struct BuddyNode {
    struct BuddyNode * parent;
    struct BuddyNode * left_child;
    struct BuddyNode * right_child;
    int size;
    int addr;
    int used;
    int order;
    int idx;
} BuddyNode;

//---------------------------------------------------------
#endif	// _memory_constants_h_
