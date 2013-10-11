/*
 *----------------------------------------------------------------------------
 * test.c
 *----------------------------------------------------------------------------
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define LOGPFX "test"
#include "debug.h"
#include "list.h"



typedef struct LINKED_List {
   int       i;
   LIST_Head l;
} LINKED_List;
LINKED_List g;

/*
 *----------------------------------------------------------------------------
 *
 * TEST_List --
 *
 *----------------------------------------------------------------------------
 */
void
TEST_List(void)
{
   LINKED_List *p;
   LIST_Head    h, *e = NULL;
   int i;

   srand (time(NULL));
   LIST_HeadInit(&h);
   for (i = 0; i < 10; i++) {
      p = malloc(sizeof(*p));
      p->i = i; //rand();
      LOG("Insert: %p %d ListEmpty %d\n", p, p->i, LIST_IsEmpty(&h));
      LIST_Insert(&h, &p->l);
   }

   while(!LIST_IsEmpty(&h)) {
      e = LIST_Remove(&h);
      p = LIST_ContainerOf(e, LINKED_List, l);
      LOG("Remove: %p %d\n", p, p->i);

   }
}

