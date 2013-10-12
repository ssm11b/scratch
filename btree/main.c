/*
 *----------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------
 */
#include <stdint.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "btree.h"
#include "btree_file.h"

#define LOGPFX "btree-util"
#include "debug.h"


static const struct option BTREE_UtilOptions[] = {
   { "file-name", required_argument,   NULL, 'f' },
   { "create",    no_argument,         NULL, 'c' },
   { "key",       required_argument,   NULL, 'k' },
   { "data",      required_argument,   NULL, 'd' },
   { "insert",    no_argument,         NULL, 'i' },
   { "query",     no_argument,         NULL, 'q' },
   { "remove",    no_argument,         NULL, 'r' },
   { "list-test", no_argument,         NULL, 'l' },
   { "help",      no_argument,         NULL, 'h' },
   { NULL,        no_argument,         NULL, 0 }
};

static const char* BTREE_UtilHelpStrings[] = {
   "btree data filename",
   "create empty btree data file",
   "key for the operation",
   "data for the operation",
   "insert [key,data] pair into the tree",
   "query [data] for the given key",
   "remove the entry for the give [key]",
   "help menu",
};

static const char *BTREE_UtilOptString = "f:k:d:liqrch?";
static const char *BTREE_FileName = NULL;
static uint64_t    BTREE_UtilOP;
static BTREE_Key   key = BTREE_INVALID;
static BTREE_Data  data = BTREE_INVALID;
static BTREE_Ops  *ops = NULL;

#define BTREE_UTIL_OP_CREATE   (1ULL<<0)
#define BTREE_UTIL_OP_INSERT   (1ULL<<1)
#define BTREE_UTIL_OP_QUERY    (1ULL<<2)
#define BTREE_UTIL_OP_REMOVE   (1ULL<<3)

extern void TEST_List(void);

/*
 *----------------------------------------------------------------------------
 *
 * BTREE_UtilUsage --
 *
 *----------------------------------------------------------------------------
 */
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

/*
 *----------------------------------------------------------------------------
 *
 * BTREE_UtilGetOptions --
 *
 *----------------------------------------------------------------------------
 */
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
         case 'c':
            BTREE_UtilOP = BTREE_UTIL_OP_CREATE;
            LOG("create\n");
            break;
         case 'k':
            key = atoll(optarg);
            break;
         case 'd':
            data = atoll(optarg);
            break;
         case 'i':
            BTREE_UtilOP = BTREE_UTIL_OP_INSERT;
            break;
         case 'q':
            BTREE_UtilOP = BTREE_UTIL_OP_QUERY;
            break;
         case 'r':
            BTREE_UtilOP = BTREE_UTIL_OP_REMOVE;
            break;
         case 'l':
            TEST_List();
            break;
         case 'h':
            LOG("help\n");
         default:
            BTREE_UtilUsage(argv[0]);
      }
      opt = getopt_long(argc, argv, BTREE_UtilOptString, BTREE_UtilOptions, &idx);
   }
}


/*
 *----------------------------------------------------------------------------
 *
 * BTREE_UtilInsert --
 *
 *----------------------------------------------------------------------------
 */
static void
BTREE_UtilInsert(void)
{
   if (key == BTREE_INVALID || data == BTREE_INVALID) {
      LOG("Invalid [key,data] = [%#lx, %#lx]\n", key, data);
   } else {
      BTREE_Init(BTREE_FileName, BTREE_FileGetOps(), 0);
      BTREE_Insert(&key, &data);
      LOG("Insert [key,data] = [%#lx, %#lx]\n", key, data);
   }
}


/*
 *----------------------------------------------------------------------------
 *
 * BTREE_UtilQuery --
 *
 *----------------------------------------------------------------------------
 */
static void
BTREE_UtilQuery(void)
{
}


/*
 *----------------------------------------------------------------------------
 *
 * BTREE_UtilRemove --
 *
 *----------------------------------------------------------------------------
 */
static void
BTREE_UtilRemove(void)
{
}


/*
 *----------------------------------------------------------------------------
 *
 * BTREE_UtilRun --
 *
 *----------------------------------------------------------------------------
 */
static void
BTREE_UtilRun(void)
{
   switch(BTREE_UtilOP) {
      case BTREE_UTIL_OP_CREATE:
         LOG("opening file-name %s\n", BTREE_FileName);
         BTREE_Init(BTREE_FileName, BTREE_FileGetOps(), 1);
         break;
      case BTREE_UTIL_OP_INSERT:
         BTREE_UtilInsert();
         break;
      case BTREE_UTIL_OP_QUERY:
         BTREE_UtilQuery();
         break;
      case BTREE_UTIL_OP_REMOVE:
         BTREE_UtilRemove();
         break;
      default:
         Panic("OP %ld not implemented.\n", BTREE_UtilOP);
         break;
   }
}


/*
 *----------------------------------------------------------------------------
 *
 * main --
 *
 *----------------------------------------------------------------------------
 */
int
main(int argc, char* argv[])
{
   BTREE_UtilGetOptions(argc, argv);
   BTREE_UtilRun();
   BTREE_Cleanup();
   return 0;
}
