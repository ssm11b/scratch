#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <assert.h>

#define ASSERT assert

#ifdef LOGPFX
#define LGPFX LOGPFX
#else
#define LGPFX ""
#endif

#define LOG(__fmt,...)  \
   printf(LGPFX ": "__fmt, ## __VA_ARGS__)

#endif // __DEBUG_H__

