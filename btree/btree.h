#ifndef __BTREE_H__
#define __BTREE_H__


typedef uint64_t  BTREE_Key;
typedef uint64_t  BTREE_Data;
typedef uint64_t  BTREE_Index;
typedef uint64_t  BTREE_Magic;
#define           BTREE_Degree   9
#define           BTREE_MAGIC    0xa5a5aa55aa55a5a5
#define           BTREE_INVALID  (-1ULL)

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
   BTREE_Key   keys[BTREE_Degree-1];
   BTREE_Key   data[BTREE_Degree-1];
   BTREE_Index nodes[BTREE_Degree];
   uint64_t    depth;
} BTREE_Node;


typedef struct BTREE_Meta {
   BTREE_Magic magic;
   BTREE_Index rootNode;
   BTREE_Index freeNode;
} BTREE_Meta;

#define BTREE_ALIGN512(x)   ((sizeof(x) + 0x1ff) & ~(0x1ff))

typedef struct BTREE_Super {
   union {
      BTREE_Meta  meta;
      uint8_t     buf[BTREE_ALIGN512(BTREE_Meta)];
   };
} BTREE_Super;

typedef struct BTREE {
   BTREE_Super*   tree;
   BTREE_Meta*    meta;
   const char*    path;
   void*          ioHandle;
} BTREE;


typedef uint64_t DEV;
typedef struct BTREE_Ops {
   int (*Open)(const char* path, int init);
   int (*WriteNode)(DEV d, BTREE_Node *b, BTREE_Index index);
   int (*ReadNode)(DEV d, BTREE_Node *b, BTREE_Index index);
   int (*WriteRaw)(DEV d, void *b, size_t sz);
   int (*ReadRaw)(DEV d, void *b, size_t sz);
   
} BTREE_Ops;


BTREE_Node* BTREE_ReadNode(BTREE_Index node);
void        BTREE_WriteNode(BTREE_Node* node);
BTREE_Index BTREE_AllocNode(BTREE* tree);
void        BTREE_FreeNode(BTREE* tree, BTREE_Index idx);

int         BTREE_Init(const char* path, BTREE_Ops *ops, int init);
int         BTREE_Insert(BTREE_Key* key, BTREE_Data *data);
int         BTREE_Find(BTREE_Key* key, BTREE_Data *data);
int         BTREE_Remove(BTREE_Key* key);

#endif // __BTREE_H__

