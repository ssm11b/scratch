#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <stdlib.h>
#include <assert.h>

#include "btree.h"


static const struct option BTREE_UtilOptions[] = {
   { "file-name", required_argument, NULL, 'f' },
   { "init", no_argument, NULL, 'i' },
   { "help", no_argument, NULL, 'h' },
   { NULL, no_argument, NULL, 0 }
};

static const char* BTREE_UtilHelpStrings[] = {
   "btree data filename",
   "create empty btree data file",
   "help menu",
};

static const char *BTREE_UtilOptString = "f:h?";
static const char *BTREE_FileName = NULL;
static uint64_t    BTREE_UtilOP;

#define BTREE_UTIL_OP_INIT   (1ULL<<0)


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
            BTREE_FileName = optarg;
            break;
         case 'i':
            BTREE_UtilOP = BTREE_UTIL_OP_INIT;
            printf("init\n");
            break;
         case 'h':
         default:
            BTREE_UtilUsage(argv[0]);
      }
      opt = getopt_long(argc, argv, BTREE_UtilOptString, BTREE_UtilOptions, &idx);
   }
}

static void
BTREE_UtilRun(void) {
   int fd;
   switch(BTREE_UtilOP) {
      case BTREE_UTIL_OP_INIT:
         printf("opening file-name %s\n", BTREE_FileName);
         fd = BTREE_FileOpen(BTREE_FileName, 1);
         assert(fd > 0);
         break;
   }
}

int
main(int argc, char* argv[]) {
   BTREE_UtilGetOptions(argc, argv);
   BTREE_UtilRun();
   return 0;
}
