#pragma once

#include "kthread.h"
#include "kthread_switch.h"
#include "../util/tree.h"


// holds internal data about a thread
typedef struct {
	smp_thread_ID id; // thread id

	// Niceness, less means higher priority. Cannot be negative
	// A thread can wait nice ticks more than another thread waiting for the same time
	unsigned int nice;

	// Last tick at which the thread started waiting
	// More than Now means the thread is actively sleeping and is not to be resumed.
	uint64_t lastTick;

	// Last-saved thread state after preemptive context switch
	smp_thread_State state;
} __smp_Thread;

// variables defined in internal.c

// current tick number
extern uint64_t __smp_Now;

// number of cores in the system
extern int __smp_Count;

// should __smp_Switch not tick once
extern bool __smp_PauseTicker;

// __smp_Thread*[], current thread for each core
extern __smp_Thread **__smp_Current;

/* Priority = lastTick + nice
 * So the average value is in fact constantly growing */

// [thread id] -> struct __smp_Thread
extern tree_Tree *__smp_Threads;
// [priority] -> struct __smp_Thread*
extern tree_Tree *__smp_ThreadsWaiting;
