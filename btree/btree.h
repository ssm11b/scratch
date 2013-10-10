#ifndef __BTREE_H__
#define __BTREE_H__



typedef uint64_t  BTREE_Key;
typedef uint64_t  BTREE_Index;
#define           BTREE_Degree 9

#define BTREE_NodeState \
   EDEF(Invalid), \
   EDEF(Init),    \
   EDEF(Inuse),   \
   EDEF(Free)

#ifdef BTREE_IMPL
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
#endif

typedef struct BTREE_Node{
   BTREE_Key      keys[BTREE_Degree-1];
   BTREE_Index   nodes[BTREE_Degree];
   uint64_t       depth;
} BTREE_Node;

typedef struct BTREE {
   BTREE_Index   rootNode;
   BTREE_Index   freeNode;
   const char*   path;
   void*         ioHandle;
} BTREE;

BTREE_Node* BTREE_ReadNode(BTREE_Index node);
void        BTREE_WriteNode(BTREE_Node* node);
BTREE_Index BTREE_AllocNode(BTREE* tree);
void        BTREE_FreeNode(BTREE* tree, BTREE_Index idx);

#endif // __BTREE_H__

