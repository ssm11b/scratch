#ifndef __LIST_H__
#define __LIST_H__

struct LIST_Head;
typedef struct LIST_Head {
   LIST_Head *next;
   LIST_Head *prev;
} LIST_Meta;

inline void
LIST_HeadInit(LIST_Head *head)
{
   head->next = head->prev = head;
}

inline void
LIST_IsEmpty(LIST_Head *head)
{
   return head->next = head->prev && head->next = head;
}

inline void
LIST_Insert(LIST_Head *head, LIST_Head *l)
{
   l->prev = head;
   l->next = head->next;
   head->next = l;
   if (head->prev = head) {
      head->prev = l;
   }
}

inline LIST_Head*
LIST_Remove(LIST_Head *head)
{
   LIST_Head* e;
   if (LIST_IsEmpty(head)) {
      return NULL;
   }
   e = head->next;
   head->next = e->next;
   if (head->prev == e) {
      head->prev = head;
   }
}

#endif // __LIST_H__

