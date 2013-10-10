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

static const char* BTREE_UtilHelpStrings[] = {
   "btree file name",
   "help menu",
};

static const char *BTREE_UtilOptString = "f:h?";
static const char *BTREE_FileName = NULL;

static void
BTREE_UtilUsage(const char* progname)
{
   int i;

   printf("usage : %s <options...>\n", progname);
   for (i = 0; i < (sizeof(BTREE_UtilOptions) / sizeof(*BTREE_UtilOptions))-1;
        i++) {
      printf("\t-%c --%-20s : %s\n", BTREE_UtilOptions[i].val,
             BTREE_UtilOptions[i].name, BTREE_UtilHelpStrings[i]);
   }
   exit(0);
}

static void
BTREE_UtilGetOptions(int argc, char* argv[]) {
   int opt;
   int idx = 0;

   opterr = 0;
   opt = getopt_long(argc, argv, BTREE_UtilOptString, BTREE_UtilOptions, &idx);
   while (opt != -1) {
      switch(opt) {
         case 'f':
            printf("file-name %s\n", optarg);
            BTREE_FileName = optarg;
            break;
         case 'h':
         default:
            BTREE_UtilUsage(argv[0]);
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
