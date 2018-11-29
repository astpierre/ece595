#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"

#define NUMBYTES 1024

void RunOSTests() {
  uint32 inode;
  int a;
  char big[NUMBYTES];
  char big2[NUMBYTES];

  inode = DfsInodeOpen("ece595-file-1");

  printf("runostests: inode after open is %d\n", inode);

  for(a=0; a<1024; a++) {
    big[a] = a;
  }

  for(a=0; a<12; a++) {
    DfsInodeWriteBytes(inode, big, a*NUMBYTES, NUMBYTES);
  }

  DfsInodeReadBytes(inode, big2, 11*NUMBYTES, NUMBYTES);

  for(a=0; a<NUMBYTES; a++) {
    if (big[a] != big2[a]) {
      printf("runostests: FAIL: index big[%d] != big2[%d] (%d != %d)\n", a, a, big[a], big2[a]);
      GracefulExit();
    }
  }

  printf("runostests: ece595-file-1 ops worked!\n");

  DfsInodeDelete(inode);

  inode = DfsInodeOpen("ece595-file-2");

  printf("runostests: ece595-file-2 open, inode = %d\n", inode);
  
}

