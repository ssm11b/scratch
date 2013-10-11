#ifndef __BTREE_H__
#define __BTREE_H__


typedef uint64_t  BTREE_Key;
typedef uint64_t  BTREE_Data;
typedef uint64_t  BTREE_Index;
typedef uint64_t  BTREE_Magic;
typedef int       BOOL;
#define           BTREE_Order    (8)
#define           BTREE_Degree   (BTREE_Order+1)
#define           BTREE_MAGIC    (0xa5a5aa55aa55a5a5)
#define           BTREE_INVALID  (-1ULL)
#define           TRUE  1
#define           FALSE 0

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

typedef struct BTREE_Node {
   BTREE_Index index;
   uint64_t    depth;
   BTREE_Key   keys[BTREE_Degree-1];
   BTREE_Data  data[BTREE_Degree-1];
   BTREE_Index nodes[BTREE_Degree];
} BTREE_Node;

// FIXME should align the node ?
#define BTREE_NODE_SZ   (sizeof(BTREE_Node))

typedef struct BTREE_Meta {
   BTREE_Magic magic;
   BTREE_Index rootNode;
   BTREE_Index freeNode;
   uint64_t    depth;
} BTREE_Meta;

#define BTREE_ALIGN512(x)   ((sizeof(x) + 0x1ff) & ~(0x1ff))

typedef struct BTREE_Super {
   union {
      BTREE_Meta  meta;
      uint8_t     buf[BTREE_ALIGN512(BTREE_Meta)];
   };
} BTREE_Super;
#define BTREE_SUPER_OFFSET (sizeof(BTREE_Super))

typedef uint64_t DEV;
typedef struct BTREE_Ops {
   int (*Open)(const char* path, int init);
   int (*WriteNode)(DEV d, BTREE_Node *b);
   int (*ReadNode)(DEV d, BTREE_Node *b);
   int (*WriteRaw)(DEV d, void *b, size_t sz, off_t off);
   int (*ReadRaw)(DEV d, void *b, size_t sz, off_t off);
} BTREE_Ops;

#if 0
typedef struct BTREE {
   BTREE_Super*   tree;
   BTREE_Meta*    meta;
   const char*    path;
   void*          ioHandle;
} BTREE;

BTREE_Node* BTREE_ReadNode(BTREE_Index node);
void        BTREE_WriteNode(BTREE_Node* node);
BTREE_Index BTREE_AllocNode(BTREE* tree);
void        BTREE_FreeNode(BTREE* tree, BTREE_Index idx);
#endif

int   BTREE_Init(const char* path, BTREE_Ops *ops, int init);
void  BTREE_Cleanup(void);
int   BTREE_Insert(BTREE_Key* key, BTREE_Data *data);
int   BTREE_Find(BTREE_Key* key, BTREE_Data *data);
int   BTREE_Remove(BTREE_Key* key);

#endif // __BTREE_H__

