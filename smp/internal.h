#pragma once

#include "kthread.h"
#include "kthread_switch.h"
#include "condiction.h"
#include "mutex.h"
#include "../util/tree.h"


typedef enum {
	__smp_Thread_Waiting_Condiction,
	__smp_Thread_Waiting_Mutex,
} __smp_Thread_Waiting_Type;

typedef struct {
	void                     *object;  // smp_Condiction* or smp_Mutex*
	void                     *data;    // Return data for Condiction, True/False for Mutex.LockTimeout
	uint64_t                  timeout; // Tick time when the wait times out, UINT64_MAX if not
	__smp_Thread_Waiting_Type type;    // Wait type
} __smp_Thread_Waiting;


// holds internal data about a thread
typedef struct {
	smp_thread_ID id; // thread id

	// Niceness, less means higher priority. Cannot be negative
	// A thread can wait nice ticks more than another thread waiting for the same time
	unsigned int nice;

	// Last tick at which the thread started waiting
	uint64_t              lastTick;
	uint64_t              sleepUntil;
	__smp_Thread_Waiting *wait;

	// Last-saved thread state after preemptive context switch
	smp_thread_State state;
	uint64_t         stackframe; // stack frame, lowest (begin) addr

	void *process; // userspace process structure, or NULL for kernel thread
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


// Returns true if the wait is waiting.
static inline bool __smp_Thread_Waiting_IsWaiting(__smp_Thread_Waiting *wait) {
	return wait && wait->object && (!wait->timeout || wait->timeout > __smp_Now);
}
