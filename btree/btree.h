#ifndef __BTREE_H__
#define __BTREE_H__



typedef uint64_t  BTREE_Key;
typedef uint64_t  BTREE_Offset;
#define           BTREE_Degree 9


#define BTREE_NodeState \
   EDEF(Invalid), \
   EDEF(Init),    \
   EDEF(Inuse),   \
   EDEF(Free)

#define EDEF(x)   x
typedef enum {
   BTREE_NodeState
} eTest;
#undef EDEF

#define EDEF(x) #x
const char *BTREE_NodeStateNames[] = {
   BTREE_NodeState
};
#undef EDEF


typedef struct BTREE_Node{
   BTREE_Key      keys[BTREE_Degree-1];
   BTREE_Offset   nodes[BTREE_Degree];
   uint64_t       depth;
} BTREE_Node;

typedef struct BTREE {
   BTREE_Offset   rootNode;
   BTREE_Offset   freeNode;
   const char*    path;
   void*          ioHandle;
} BTREE;

BTREE_Node*    BTREE_ReadNode(BTREE_Offset node);
void           BTREE_WriteNode(BTREE_Node* node);
BTREE_Offset   BTREE_AllocNode(BTREE* tree);
void           BTREE_FreeNode(BTREE* tree, BTREE_Offset node);

#endif // __BTREE_H__

