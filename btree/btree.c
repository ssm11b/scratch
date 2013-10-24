/*
 *----------------------------------------------------------------------------
 * btree.c
 *----------------------------------------------------------------------------
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "btree.h"
#define LOGPFX "btree"
#include "debug.h"

BTREE_Super* tree;
BTREE_Meta*  meta;
DEV          dev;
BTREE_Node*  root;
BTREE_Ops*   btops;

#define FOR_EACH_KEY_COND(idx, n, c)  \
   for (int idx = 0; idx < BTREE_Order && c; idx++) 

#define FOR_EACH_KEY(idx, n) FOR_EACH_KEY_COND(idx, n, 1)
#define ROF_EACH_KEY

#define FOR_EACH_BTREE_NODE(idx, n) \
   for (int idx = 0; idx < BTREE_Degree; idx++)
#define ROF_EACH_BTREE_NODE(idx, n) \

#define BTREE_NODE_ISLEAF(n)  (meta->depth == n->depth)


#define BTREE_OP  \
   EDEF(INVALID), \
   EDEF(INIT),    \
   EDEF(INSERT),  \
   EDEF(REMOVE),  \
   EDEF(FIND)

#define EDEF(x)   x
typedef enum {
   BTREE_OP
} BTREEOP;
#undef EDEF

#define EDEF(x) #x
const char *BTREE_OPNames[] = {
   BTREE_OP
};
#undef EDEF

#define LOGMASK(l) (1 << l)
int BTREE_LogMask = LOGMASK(FIND) | LOGMASK(INIT);
#define BTREE_LOG(level, __fmt,...)                                        \
   if (BTREE_LogMask & 1<<level) {                                         \
      LOG("%-7s %-15s "__fmt, BTREE_OPNames[level], __func__, ## __VA_ARGS__);  \
   }



/*
 *----------------------------------------------------------------------------
 *
 * BTREEInitNode --
 *
 *    Allocate and initialize the BTREE_FileNode, if clean is specified
 *    invalidate the keys.
 *
 *----------------------------------------------------------------------------
 */
static BTREE_Node*
BTREEInitNode(BTREE_Index index, uint64_t depth, BOOL clean)
{
   BTREE_Node* n = malloc(sizeof(*n));

   BTREE_LOG(INIT, "index %#lx depth %#lx clean %d\n", index, depth, clean);

   ASSERT(n != NULL);
   memset(n, 9, sizeof(*n));
   n->fn = malloc(sizeof(*n->fn));
   ASSERT(n->fn != NULL);
   FOR_EACH_KEY_COND(i, n->fn, clean) {
      n->fn->keys[i] = BTREE_INVALID;
   } ROF_EACH_KEY;
   n->fn->index = index;
   n->fn->depth = depth;
   return n;
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
      btops->WriteNode(dev, n->fn);
   }
   return n;
}


static BTREE_Node*
BTREEInitNodeFromDev(BTREE_Index index, uint64_t depth)
{
   BTREE_Node *n = BTREEInitNode(index, depth, FALSE);
   btops->ReadNode(dev, n->fn);
   return n; 
}

static BTREE_Node*
BTREEGetNode(BTREE_Node* n, BTREE_Index index) 
{
   ASSERT(index < BTREE_Degree);
   if (n->nodes[index] == NULL) {
      n->nodes[index] = BTREEInitNodeFromDev(index, n->fn->depth + 1); 
   }
   return n->nodes[index];
}


/*
 *----------------------------------------------------------------------------
 *
 * BTREEFIndIndex --
 *
 *    Find the index for the given key in the BTREE_Node.
 *
 * Return:
 *    If key exists :
 *       return TRUE and BTREE_Index for the key in found
 *    else :
 *       return FALSE the BTREE_Index of the child BTREE_FileNode where
 *       the key should exist.
 *
 *----------------------------------------------------------------------------
 */
static BOOL
BTREEFindIndex(BTREE_Node* n, BTREE_Key* key, BTREE_Index *found, int *pos)
{
   FOR_EACH_KEY(i, n) {
      if (n->fn->keys[i] == BTREE_INVALID || *key <= n->fn->keys[i]) {
         *pos = i;
         *found = n->fn->nodes[i];
         return n->fn->keys[i] == *key;
      }
   } ROF_EACH_KEY;
   *pos = BTREE_Order;
   *found = n->fn->nodes[BTREE_Order];
   return FALSE; 
}


/*
 *----------------------------------------------------------------------------
 *
 * BTREEFind --
 *
 *----------------------------------------------------------------------------
 */
static BTREE_Node*
BTREEFind(BTREE_Node* n, BTREE_Key* key, BTREE_Index *found)
{
   int pos;
   BOOL exists = BTREEFindIndex(n, key, found, &pos);
   BTREE_LOG(FIND, "key %#lx exists %d found %#lx\n", *key, exists, *found);
   if (exists) {
      return n;
   } else if (BTREE_NODE_ISLEAF(n->fn)) {
      return NULL;
   }
   /* Load the child node if needed and recursively find the node for
    * that contains the key.
    */
   return BTREEFind(BTREEGetNode(n, pos), key, found);
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
   BTREE_Node *n;
   BTREE_Index idx;
   ASSERT(meta != NULL);

   BTREE_LOG(FIND, "key %#lx\n", *key);
   n = BTREEFind(root, key, &idx);
   if (n == NULL) {
      return -EINVAL;
   }
   ASSERT(idx < BTREE_Order);
   *data = n->fn->data[idx];
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
      meta->rootNode = meta->freeNode;
      meta->freeNode++;
      ASSERT(btops->WriteRaw(dev, meta, sizeof(BTREE_Super), 0));
      ASSERT(root != NULL);

   } else {
      ASSERT(btops->ReadRaw(dev, meta, sizeof(BTREE_Super), 0));
      root = BTREEInitNodeFromDev(meta->rootNode, meta->depth);
   }
   BTREE_LOG(INIT, "magic %#lx root %#lx free %#lx\n", meta->magic,
             meta->rootNode, meta->freeNode);
   return 0;
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
