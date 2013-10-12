/*
 *----------------------------------------------------------------------------
 * btree_file.c
 *----------------------------------------------------------------------------
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include "btree.h"
#include "debug.h"

/*
 *----------------------------------------------------------------------------
 *
 * BTREE_FileOpen --
 *
 *----------------------------------------------------------------------------
 */
int BTREE_FileOpen(const char* file, int init) {
   int flags = O_RDWR | O_CREAT | (init ? O_TRUNC : 0);
   return file == NULL ? -EINVAL :
          open(file, flags, S_IRUSR|S_IWUSR);
}

/*
 *----------------------------------------------------------------------------
 *
 * BTREE_FileWriteNode --
 *
 *----------------------------------------------------------------------------
 */
int BTREE_FileWriteNode(DEV fd, BTREE_Node *n)
{
   off_t offset = (BTREE_NODE_SZ * n->index) + BTREE_SUPER_OFFSET;
   ASSERT(n != NULL);
   lseek(fd, offset, SEEK_SET);
   return write(fd, n, sizeof(*n)) == sizeof(*n);
}

/*
 *----------------------------------------------------------------------------
 *
 * BTREE_FileReadNode --
 *
 *----------------------------------------------------------------------------
 */
int BTREE_FileReadNode(DEV fd, BTREE_Node *b) {
   return 0;
}

/*
 *----------------------------------------------------------------------------
 *
 * BTREE_FileWriteRaw --
 *
 *----------------------------------------------------------------------------
 */
int BTREE_FileWriteRaw(DEV fd, void *b, size_t sz, off_t off) {
   lseek(fd, off, SEEK_SET);
   return write(fd, b, sz);
}

/*
 *----------------------------------------------------------------------------
 *
 * BTREE_FileReadRaw --
 *
 *----------------------------------------------------------------------------
 */
int BTREE_FileReadRaw(DEV fd, void *b, size_t sz, off_t off) {
   lseek(fd, off, SEEK_SET);
   return read(fd, b, sz);
}

BTREE_Ops BTREE_FileOps = {
   .Open       = BTREE_FileOpen,
   .WriteNode  = BTREE_FileWriteNode,
   .ReadNode   = BTREE_FileReadNode,
   .WriteRaw   = BTREE_FileWriteRaw,
   .ReadRaw    = BTREE_FileReadRaw,
};

/*
 *----------------------------------------------------------------------------
 *
 * BTREE_FileGetOps --
 *
 *----------------------------------------------------------------------------
 */
BTREE_Ops* BTREE_FileGetOps(void)
{
   return &BTREE_FileOps;
}
