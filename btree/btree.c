
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "btree.h"
#define LOGPFX "btree"
#include "debug.h"

static BTREE tree;

int BTREE_Insert(BTREE_Key* key, BTREE_Data *data)
{
   return 0;
}


int BTREE_Find(BTREE_Key* key, BTREE_Data *data)
{
   return 0;
}


int BTREE_Remove(BTREE_Key* key)
{
   return 0;

}

int BTREE_Init(const char* path, BTREE_Ops *ops, int init)
{
   int         fd = ops->Open(path, init);
   BTREE_Meta* m  = malloc(sizeof(BTREE_Super));

   if (init) {
      ASSERT(fd > 0);
      ASSERT(m != NULL);
      memset(m, 0xa5, sizeof(BTREE_Super));
      m->magic    = BTREE_MAGIC;
      m->rootNode = BTREE_INVALID;
      m->freeNode = 0;

      ASSERT(ops->WriteRaw(fd, m, sizeof(BTREE_Super)));
   } else {
      ASSERT(ops->ReadRaw(fd, m, sizeof(BTREE_Super)));
   }

   LOG("magic %#lx root %#lx free %#lx\n", m->magic,
       m->rootNode, m->freeNode);
   tree.meta = m;
}
