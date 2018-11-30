#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"
#include "misc.h"

void RunOSTests() 
{
    char writeclass[6] = "ece595";
    char readclass[6];
    uint32 file_handle;
    uint32 i=0;
 
    printf("\n\n");
    printf("============================================================\n");
    printf(" ostests.c (PID: %d) Beginning DFS driver tests...\n");
    printf("============================================================\n");
    printf("   fhandle      =    DfsInodeOpen('andrew')\n");
    file_handle = DfsInodeOpen("andrew");
    printf("   fhandle      =    %d\n",file_handle);
    printf("   fsize        =    %d bytes\n",DfsInodeFilesize(file_handle));
    printf("============================================================\n");
    printf("  Attempting to write the course title to file 'andrew'...\n");
    printf("   base_addr    =    %d\n",20);
    printf("   nbytes       =    %d\n",6);
    printf("   str          =    ece595\n");
    printf("   DfsInodeWriteBytes(fhandle, str, base_addr, nbytes)\n");
    DfsInodeWriteBytes(file_handle, &writeclass, 20, 6);
    printf("  Now let's check filesize of 'andrew'... (expecting 26)\n");
    printf("   fsize        =    %d bytes\n",DfsInodeFilesize(file_handle));

    printf("  Attempting to read the course title from file 'andrew'...\n");
    printf("   base_addr    =    %d\n",20);
    printf("   nbytes       =    %d\n",6);
    printf("   DfsInodeReadBytes(fhandle, buff, base_addr, nbytes)\n");
    DfsInodeReadBytes(file_handle, &readclass, 20, 6);
    printf("  Read from file:  ");
    for(i=0;i<6;i++) printf("%c",readclass[i]);
    printf("\n");
    
    printf("  Attempting to delete the file 'andrew'... DfsInodeDelete()\n");
    DfsInodeDelete(file_handle);
    printf("  Now, having deleted the file 'andrew', check existence...\n");
    if(DfsInodeFilenameExists("andrew") == DFS_FAIL)
    {  printf("   filename: andrew, NOT FOUND IN FILESYSTEM!\n");  }
    else printf("   filename: andrew, OH NO, STILL EXISTS!\n");
    printf("  Now let's reopen 'andrew' and check filesize... (expecting 0)\n");
    file_handle = DfsInodeOpen("andrew");
    printf("   fsize        =    %d bytes\n",DfsInodeFilesize(file_handle));

    printf("============================================================\n");
    printf("============================================================\n\n");
}

