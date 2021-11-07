
#include "kthread.h"
#include "kthread_switch.h"
#include "internal.h"

#include "../runtime/stdio.h"
#include <string.h>

#include "../util/tree_internal.h"


// defined in assembly
SYSV_ABI void __smp_Switch_Idle();

#define NODE_POINTER(node) (*((__smp_Thread **)(node->data)))

static void printTree(tree_Node *root, int level, __tree_ConnectType type) {
	if (!root)
		return;
	for (int i = 0; i < level * 4; i++)
		io_WriteConsoleASCII(" ");
	io_Printf("%s, key=%d, data=%llx, &data=%llx\n", type == __tree_Left ? "Left" : "Right", root->key, (uint64_t)NODE_POINTER(root), root->data);
	printTree(root->left, level + 1, __tree_Left);
	printTree(root->right, level + 1, __tree_Right);
}

SYSV_ABI uintptr_t __smp_Switch() {
	// the calling function smp_IntSwitch already CLI-ed for us

	// TODO ticker and switch should be 2 functions!
	if (!__smp_PauseTicker) {
		__smp_Now++;
		if (__smp_Now % 8)
			return 0;
	} else
		__smp_PauseTicker = false;

	//io_Printf("__smp_Switch: Tick: %d, switching\n", __smp_Now);

	__smp_Thread *t = __smp_Current[0];

	tree_Node *node = 0;
	// insert the current thread back into the waiting queue
	if (t) {
		uint64_t priority = t->nice + (t->lastTick > __smp_Now ? t->lastTick : __smp_Now); // new priority for the thread
		bool     ok       = false;
		do {
			node = tree_InsertNode(__smp_ThreadsWaiting, priority, &ok);
			priority++;
		} while (!ok);
		NODE_POINTER(node) = t;
	}
	//printTree(__smp_Threads->root, 0, 0);
	//printTree(__smp_ThreadsWaiting->root, 0, 0);
	tree_Node *first = tree_FirstNode(__smp_ThreadsWaiting);

	//io_Printf("    first0.id=%d\n", NODE_POINTER(first) ? NODE_POINTER(first)->id : 0);
	while (first && NODE_POINTER(first)->lastTick > __smp_Now) {
		//io_Printf("      iterating, .id=%d\n", NODE_POINTER(first) ? NODE_POINTER(first)->id : 0);
		first = tree_Node_Next(first);
	}

	if (first == node) {
		// the current thread is still the first, return
		//io_Printf("    Not context switching, still running %d\n", t ? t->id : 0);
		if (node)
			tree_Delete(__smp_ThreadsWaiting, node);
		return 0;
	}

	// we need a real context switch
	// first save the current thread context
	t->lastTick = __smp_Now;
	memcpy(&t->state, &__smp_IntSwitch_LastState, sizeof(smp_thread_State));

	if (!first) {
		// no thread available, load a dummy idle thread
		__smp_IntSwitch_LastState.rip = (uint64_t)__smp_Switch_Idle;
		__smp_Current[0]              = 0;
		io_WriteConsoleASCII("__smp_Switch: Entering idle\n");
	} else {
		// load the new context
		io_Printf("    Context switching, from %d to %d\n", t ? t->id : 0, NODE_POINTER(first)->id);
		memcpy(&__smp_IntSwitch_LastState, &NODE_POINTER(first)->state, sizeof(smp_thread_State));
		__smp_Current[0] = NODE_POINTER(first);
		tree_Delete(__smp_ThreadsWaiting, first);
	}

	return 1;
}
