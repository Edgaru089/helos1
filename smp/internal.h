#pragma once

#include "kthread.h"
#include "kthread_switch.h"
#include "condiction.h"
#include "../util/tree.h"


// holds internal data about a thread
typedef struct {
	smp_thread_ID id; // thread id

	// Niceness, less means higher priority. Cannot be negative
	// A thread can wait nice ticks more than another thread waiting for the same time
	unsigned int nice;

	// Last tick at which the thread started waiting
	uint64_t       lastTick;
	uint64_t       sleepUntil;
	smp_Condition *waitCondition;
	void *         waitData;

	// Last-saved thread state after preemptive context switch
	smp_thread_State state;
	uint64_t         stackframe; // stack frame, lowest (begin) addr
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

// largest thread ID
extern smp_thread_ID __smp_Idallo;

/* Priority = lastTick + nice
 * So the average value is in fact constantly growing */

// [thread id] -> struct __smp_Thread
extern tree_Tree *__smp_Threads;
// [priority] -> struct __smp_Thread*
extern tree_Tree *__smp_ThreadsWaiting;

// unused thread stack pages for use; 4K in size
extern tree_Tree *__smp_StackPool;
