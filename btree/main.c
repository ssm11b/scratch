
#include <stdint.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "btree.h"

#define LOGPFX "btree-util"
#include "debug.h"


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

static const char *BTREE_UtilOptString = "f:ih?";
static const char *BTREE_FileName = NULL;
static uint64_t    BTREE_UtilOP;

#define BTREE_UTIL_OP_INIT   (1ULL<<0)


static void
BTREE_UtilUsage(const char* progname)
{
   int i;

   LOG("usage : %s <options...>\n", progname);
   for (i = 0; i < (sizeof(BTREE_UtilOptions) / sizeof(*BTREE_UtilOptions))-1;
        i++) {
      LOG("\t-%c --%-20s : %s\n", BTREE_UtilOptions[i].val,
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
            LOG("init\n");
            break;
         case 'h':
            LOG("help\n");
         default:
            BTREE_UtilUsage(argv[0]);
      }
      opt = getopt_long(argc, argv, BTREE_UtilOptString, BTREE_UtilOptions, &idx);
   }
}


static void
BTREE_UtilInit(void) {
   int         fd = BTREE_FileOpen(BTREE_FileName, 1);
   BTREE_Meta* m  = malloc(sizeof(BTREE_Super));

   ASSERT(fd > 0);
   ASSERT(m != NULL);
   memset(m, 0xa5, sizeof(BTREE_Super));
   m->magic    = BTREE_MAGIC;
   m->rootNode = BTREE_INVALID;
   m->freeNode = 0;

   ASSERT(BTREE_FileWriteRaw(fd, m, sizeof(BTREE_Super)));
   free(m);
}


static void
BTREE_UtilRun(void) {
   switch(BTREE_UtilOP) {
      case BTREE_UTIL_OP_INIT:
         LOG("opening file-name %s\n", BTREE_FileName);
         BTREE_UtilInit();
         break;
   }
}

int
main(int argc, char* argv[]) {
   BTREE_UtilGetOptions(argc, argv);
   BTREE_UtilRun();
   return 0;
}
