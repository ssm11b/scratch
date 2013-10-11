/*
 *----------------------------------------------------------------------------
 * btree.c
 *----------------------------------------------------------------------------
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "btree.h"
#define LOGPFX "btree"
#include "debug.h"

BTREE_Super* tree;
BTREE_Meta*  meta;
DEV          dev;
BTREE_Node*  root;
BTREE_Ops*   btops;


/*
 *----------------------------------------------------------------------------
 *
 * BTREEInitNode --
 *
 *----------------------------------------------------------------------------
 */
static BTREE_Node*
BTREEInitNode(BTREE_Index index, uint64_t depth, BOOL clean)
{
   BTREE_Node *n = malloc(sizeof(*n));
   int i;
   for (i = 0; i < BTREE_Order && clean; i++) {
      n->keys[i] = BTREE_INVALID;
   }
   n->index = index;
   n->depth = depth;
}

/*
 *----------------------------------------------------------------------------
 *
 * BTREEAllocNode --
 *
 *----------------------------------------------------------------------------
 */
static BTREE_Node*
BTREEAllocNode(BTREE_Index index, uint64_t depth, BOOL sync)
{
   BTREE_Node *n = BTREEInitNode(index, depth, TRUE);
   if (sync) {
      btops->WriteNode(dev, n);
   }
   return n;
}

/*
 *----------------------------------------------------------------------------
 *
 * BTREEFindNode --
 *
 *----------------------------------------------------------------------------
 */
static BTREE_Node*
BTREEFindNode(BTREE_Node* n, BTREE_Key* key)
{
   ASSERT(n != NULL);
}

/*
 *----------------------------------------------------------------------------
 *
 * BTREE_Insert --
 *
 *----------------------------------------------------------------------------
 */
int
BTREE_Insert(BTREE_Key* key, BTREE_Data *data)
{
   ASSERT(meta != NULL);

   return 0;
}


/*
 *----------------------------------------------------------------------------
 *
 * BTREE_Find --
 *
 *----------------------------------------------------------------------------
 */
int
BTREE_Find(BTREE_Key* key, BTREE_Data *data)
{
   ASSERT(meta != NULL);
   return 0;
}


/*
 *----------------------------------------------------------------------------
 *
 * BTREE_Remove --
 *
 *----------------------------------------------------------------------------
 */
int
BTREE_Remove(BTREE_Key* key)
{
   ASSERT(meta != NULL);
   return 0;

}

/*
 *----------------------------------------------------------------------------
 *
 * BTREE_Init --
 *
 *    Initializes the module for use.  If init is TRUE, this module will
 *    create a empty BTREE state on the device.  Once this method returns
 *    the BTREE meta data and root node will be resident in memory.
 *
 *----------------------------------------------------------------------------
 */
int
BTREE_Init(const char* path, BTREE_Ops *o, BOOL init)
{
   btops = o;
   dev = btops->Open(path, init);
   meta = malloc(sizeof(BTREE_Super));

   if (init) {
      ASSERT(dev > 0);
      ASSERT(meta != NULL);
      memset(meta, 0xa5, sizeof(BTREE_Super));
      meta->magic    = BTREE_MAGIC;
      meta->rootNode = BTREE_INVALID;
      meta->freeNode = 0;
      meta->depth    = 0;

      root = BTREEAllocNode(meta->freeNode, meta->depth, TRUE);
      meta->freeNode++;
      ASSERT(btops->WriteRaw(dev, meta, sizeof(BTREE_Super), 0));
      ASSERT(root != NULL);

   } else {
      ASSERT(btops->ReadRaw(dev, meta, sizeof(BTREE_Super), 0));
      root = BTREEInitNode(meta->freeNode, meta->depth, FALSE);
      btops->ReadNode(dev, root);
   }
   LOG("magic %#lx root %#lx free %#lx\n", meta->magic,
       meta->rootNode, meta->freeNode);
}


/*
 *----------------------------------------------------------------------------
 *
 * BTREE_Cleanup --
 *
 *    Called to clean resources used by this module,  does so in the following
 *    manner:
 *       - write any dirty state back to device.
 *       - close deivce.
 *       - free memory resources.
 *
 *----------------------------------------------------------------------------
 */
void BTREE_Cleanup(void)
{
   // write state back to device

   // close device

   // free memory
   free(root);
   free(meta);
}
