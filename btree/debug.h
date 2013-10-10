#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <assert.h>


#ifdef LOGPFX
#define LGPFX LOGPFX
#else
#define LGPFX ""
#endif

#define LOG(__fmt,...)  \
   printf(LGPFX ": "__fmt, ## __VA_ARGS__)

#define ASSERT assert
#define Panic(fmt,...)                 \
   LOG("Panic: "fmt, ## __VA_ARGS__);  \
   ASSERT(0)



#endif // __DEBUG_H__

