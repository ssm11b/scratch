#ifndef __BTREE_FILE_H__
#define __BTREE_FILE_H__

int BTREE_FileOpen(const char* filename);
int BTREE_FileWriteNode(int fd, BTREE_Node *b, BTREE_Idx index);
int BTREE_FileReadNode(int fd, BTREE_Node *b, BTREE_Idx index);
int BTREE_FileWriteRaw(int fd, char *b, size_t sz);
int BTREE_FileReadRaw(int fd, char *b, size_t sz);

#endif // __BTREE_FILE_H__

