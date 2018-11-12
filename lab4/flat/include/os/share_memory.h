/***********************************************************************
 *
 * share_memory.h
 * Copyright: Uday Savagaonkar (2002). All rights reserved.
 * Share memory API for dlxos
 *
 ***********************************************************************/

#ifndef __SHARE_MEMORY_H__
#define __SHARE_MEMORY_H__

#include "dlxos.h"
#include "memory.h"
#include "process.h"

#define MEMORY_MAX_SHARED_PAGES 32 //Maximum number of pages that can be
				   //shared amongst different processes
#define PROCESS_MAX_PAGES 16	   //Maximum number of pages allowed in Level
				   //1 page table

void SharedInitModule();	//Turns on the shared memory module
uint32 MemoryCreateSharedPage(PCB *pcb);
				//Creates a shared page in the memory
void *mmap(PCB *pcb, uint32 handle);
				//Maps a shared page to the virtual address
				//space
int MemoryFreeSharedPage(PCB *pcb, uint32 handle);
				//Releases a shared page. The page is not
				//actually deallocated untill all the process
				//release it.
int MemoryFreeSharedPte(PCB *pcb, int pagenumber); // called in process.c when freeing resources

void ShareModuleInit(); // Initializes shared memory

#endif

