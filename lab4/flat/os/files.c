#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

// STUDENT: put your file-level functions here
uint32 FileOpen(char *filename, char *mode) {
  return -1;
}

int FileClose(uint32 handle) {
  return FILE_FAIL;
}

int FileRead(uint32 handle, void *mem, int num_bytes) {
  return FILE_FAIL;
}

int FileWrite(uint32 handle, void *mem, int num_bytes) {
  return FILE_FAIL;
}

int FileSeek(uint32 handle, int num_bytes, int from_where) {
  return FILE_FAIL;
}

int FileDelete(char *filename) {
  return FILE_FAIL;
}
