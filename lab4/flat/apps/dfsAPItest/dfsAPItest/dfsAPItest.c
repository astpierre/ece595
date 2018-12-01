#include "usertraps.h"
#include "misc.h"
#include "files_shared.h"

void main (int argc, char *argv[])
{
    // Variable declarations
    int handle, i;
    char * fname= "astpier-file1";
    char * strout = " ECE595LAB4";
    char rbuff[11];

    Printf("");
    Printf("\n\n"); 
    Printf("============================================================\n"); 
    Printf(" dfsAPItest.c (PID: %d): Q6, user program to test DFS API\n", getpid()); 
    Printf("============================================================\n"); 

    // 1. Lets create a file, "astpier-file1"
    Printf("  Opening file \"astpier-file1\"... file_open(\"astpier-file1\", \"w\")\n"); 
    handle = file_open(fname,"w");
    if(handle == FILE_FAIL) Printf(" dfsAPItest.c : FILE OPEN FOR WRITE FAILED\n");

    // 2. Write "testStr" to this file
    Printf("  Writing to file \"astpier-file1\"... file_write(fhandle, \"ECE595LAB4\", dstrlen(\"ECE595LAB4\"))\n");
    i = file_write(handle,strout,11);
    if(i == FILE_FAIL) Printf(" dfsAPItest.c : FILE WRITE FAILED\n");
    
    // 3. Close the file
    Printf("  Closing file \"astpier-file1\"... file_close(fhandle)\n");
    i = file_close(handle);
    if(i == FILE_FAIL) Printf(" dfsAPItest.c : FILE CLOSE FAILED\n");
    
    // 4. Re-open the file, "andrewsFile"
    Printf("  Re-opening file \"astpier-file1\"... file_open(\"astpier-file1\", \"r\")\n"); 
    handle = file_open(fname, "r");
    if(handle == FILE_FAIL) Printf(" dfsAPItest.c : FILE OPENING FOR READ FAILED \n");
 
    // 5. Read strlen(testStr) bytes from file (should match testStr)
    Printf("  Reading from file \"astpier-file1\"... file_read(fhandle, buffer, dstrlen(\"ECE595LAB4\"))\n");
    i = file_read(handle,rbuff,11);
    if(i == FILE_FAIL) Printf(" dfsAPItest.c : FILE READ FAILED\n");
    Printf("   Read from file:  %s\n",rbuff);
    
    Printf("============================================================\n"); 
    Printf(" dfsAPItest.c (PID: %d): DFS API test complete, process ending...\n", getpid()); 
    Printf("============================================================\n"); 
    Printf("\n\n"); 
}
