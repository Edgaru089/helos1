#pragma once

#include "../main.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


// Thread is a kernel thread primitive.
typedef int smp_thread_ID;

// Arguments are the numeric arguments passed to the thread entry point.
typedef struct {
	uintptr_t a, b, c, d, e, f;
} smp_thread_Arguments;

// Init initializes the kernel multithreading environment.
//
// The current execution environment is saved as a thread, with its ID returned (usually 1).
smp_thread_ID smp_thread_Init();

// Start starts a new thread, putting it into the scheduler (but not executing it right off)
//
// The entry point specified must have System V x64 ABI,
// and only up to 6 numeric arguments.
//
// The return value is discarded.
smp_thread_ID smp_thread_Start(void *entry, const smp_thread_Arguments *args, unsigned int nice);

// Nice sets the new niceness for a thread.
//
// If newnice is less than 0, the new niceness is not set.
//
// Returns the old niceness of the thread.
// If the thread does not exist, -1 is returned.
int smp_thread_Nice(smp_thread_ID id, int newnice);

// Yield pauses the execution of the current thread, possibly switching to another.
SYSV_ABI void smp_thread_Yield();

// Sleep sleeps for a given amount of ticks (1024Hz)
void smp_thread_Sleep(int ticks);

// Default niceness for kernel threads.
#define SMP_NICENESS_DEFAULT 80


#ifdef __cplusplus
}
#endif
