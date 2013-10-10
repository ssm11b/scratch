
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "btree.h"

int BTREE_FileOpen(const char* file, int init) {
   int flags = O_RDWR | O_CREAT | (init ? O_TRUNC : 0);
   return file == NULL ? -EINVAL :
          open(file, flags, S_IRUSR|S_IWUSR);
}

int BTREE_FileWrite(int fd, BTREE_Node *b, BTREE_Index index) {
   return 0;
}

int BTREE_FileRead(int fd, BTREE_Node *b, BTREE_Index index) {
   return 0;
}

int BTREE_FileWriteRaw(int fd, char *b, size_t sz) {
   return write(fd, b, sz);
}

int BTREE_FileReadRaw(int fd, char *b, size_t sz) {
   return read(fd, b, sz);
}
