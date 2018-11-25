#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"
#include "misc.h"

void RunOSTests() 
{
    char myname[] = "andrew";
    char * classinfo = "ece595";
    uint32 file_handle;
    uint32 tmp=0;
 
    printf("\n\n");
    printf("============================================================\n");
    printf(" ostests.c (PID: %d) Beginning DFS driver tests...\n");
    printf("============================================================\n");
    printf("   file_handle  =    DfsInodeOpen('andrew')\n");
    file_handle = DfsInodeOpen("andrew");
    printf("   file_handle  =    %d\n",file_handle);
    printf("   file-size    =    %d bytes\n",DfsInodeFilesize(file_handle));
    printf("============================================================\n");
    printf("   Attempting to write the course title to file 'andrew'...\n");
    printf("   DfsInodeWriteBytes(file_handle, classinfo, 0, dstrlen(classinfo)+1)\n");
    DfsInodeWriteBytes(file_handle, classinfo, 512, 6);
    printf("   file-size    =    %d bytes\n",DfsInodeFilesize(file_handle));
    printf("============================================================\n");
    printf("============================================================\n\n");
    return;
}

