#pragma once

#include "../../runtime/panic_assert.h"
#include "../../memory/memory.h"
#include "../../memory/heap_break.h"

// Added: dlmalloc build config file


#ifdef _WIN32
#undef _WIN32 // We're not compiling for Win32 even on MinGW
#endif

#ifdef WIN32
#undef WIN32
#endif

#define USE_DL_PREFIX                                // Use dlmalloc() instead of malloc(), etc.
#define USE_LOCKS               0                    // kMalloc() and fellow handles locking and interrupt
#define HAVE_MORECORE           1                    // Have sbrk() clone
#define MORECORE                memory_AddBreak      // sbrk() name
#define MORECORE_CANNOT_TRIM                         // sbrk() does not handle negative increments
#define HAVE_MMAP               0                    // Does not have mmap()
#define malloc_getpagesize      SYSTEM_PAGE_SIZE     // System page size
#define MALLOC_FAILURE_ACTION                        // Do nothing on failure
#define NO_MALLOC_STATS         1                    // To avoid fprintf()
#define ABORT_ON_ASSERT_FAILURE 0                    // Use our own assert() marco
#define ABORT                   __Panic_HaltSystem() // Use our own abort()

#define LACKS_UNISTD_H
#define LACKS_FCNTL_H
#define LACKS_SYS_PARAM_H
#define LACKS_SYS_MMAN_H
//#define LACKS_STRINGS_H
//#define LACKS_STRING_H
#define LACKS_SYS_TYPES_H
//#define LACKS_ERRNO_H
//#define LACKS_STDLIB_H
#define LACKS_SCHED_H
#define LACKS_TIME_H
