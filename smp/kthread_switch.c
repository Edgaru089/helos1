
#include "kthread.h"
#include "kthread_switch.h"
#include "internal.h"

#include "../runtime/stdio.h"
#include <string.h>


// defined in assembly
SYSV_ABI void __smp_Switch_Idle();

SYSV_ABI uintptr_t __smp_Switch() {
	// the calling function smp_IntSwitch already CLI-ed for us

	// TODO ticker and switch should be 2 functions!
	if (!__smp_PauseTicker) {
		__smp_Now++;
		if (__smp_Now % 8)
			return 0;
	} else
		__smp_PauseTicker = false;

	io_Printf("__smp_Switch: Tick: %d, switching\n", __smp_Now);

	__smp_Thread *t = __smp_Current[0];

	tree_Node *node = 0;
	// insert the current thread back into the waiting queue
	if (!t) {
		uint64_t priority = t->nice + __smp_Now; // new priority for the thread
		node              = tree_InsertNode(__smp_ThreadsWaiting, priority, 0);
	}
	tree_Node *first = tree_FirstNode(__smp_ThreadsWaiting);

	while (first && (*((__smp_Thread **)node->data))->lastTick > __smp_Now)
		first = tree_Node_Next(first);

	if (first == node) {
		// the current thread is still the first, return
		io_Printf("    Not context switching, still running %d\n", t ? t->id : 0);
		if (!t)
			tree_Delete(__smp_ThreadsWaiting, node);
		return 0;
	}

	// we need a real context switch
	// first save the current thread context
	*((void **)node->data) = t;
	t->lastTick            = __smp_Now;
	memcpy(&t->state, &__smp_IntSwitch_LastState, sizeof(smp_thread_State));

	if (!first) {
		// no thread available, load a dummy idle thread
		__smp_IntSwitch_LastState.rip = (uint64_t)__smp_Switch_Idle;
		__smp_Current[0]              = 0;
		io_WriteConsoleASCII("__smp_Switch: Entering idle\n");
	} else {
		// load the new context
		io_Printf("    Context switching, from %d to %d\n", t ? t->id : 0, ((__smp_Thread *)first->data) ? ((__smp_Thread *)first->data)->id : 0);
		memcpy(&__smp_IntSwitch_LastState, &((__smp_Thread *)first->data)->state, sizeof(smp_thread_State));
		tree_Delete(__smp_ThreadsWaiting, first);
		__smp_Current[0] = (__smp_Thread *)first->data;
	}

	return 1;
}
