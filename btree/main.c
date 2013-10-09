#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <stdlib.h>

#include "btree.h"



static const struct option BTREE_UtilOptions[] = {
   { "file-name", required_argument, NULL, 'f' },
   { "help", no_argument, NULL, 'h' },
   { NULL, no_argument, NULL, 0 }
};

static const char *BTREE_UtilOptString = "f:h?";

static void
BTREE_UtilGetOptions(int argc, char* argv[]) {
   int opt;
   int idx = 0;

   opterr = 0;
   opt = getopt_long(argc, argv, BTREE_UtilOptString, BTREE_UtilOptions, &idx);
   while (opt != -1) {
      switch(opt) {
         case 'f':
            break;
         case 'h':
            break;
         default:
            abort();
      }
      opt = getopt_long(argc, argv, BTREE_UtilOptString, BTREE_UtilOptions, &idx);
   }
}

int
main(int argc, char* argv[]) {
   BTREE_UtilGetOptions(argc, argv);
   printf("BTREE %#lx BTREE_Node %#lx\n",
          sizeof(BTREE), sizeof(BTREE_Node));
   return 0;
}
