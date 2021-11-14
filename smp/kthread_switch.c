
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


	__smp_Thread *t = __smp_Current[0];

	uint64_t priority = UINT64_MAX;
	if (t)
		priority = t->nice + (t->lastTick > __smp_Now ? t->lastTick : __smp_Now); // new priority for the thread

	// find the waiting thread with the least priority
	tree_Node *first = tree_FirstNode(__smp_ThreadsWaiting);
	while (first && (NODE_POINTER(first)->sleepUntil > __smp_Now || NODE_POINTER(first)->waitCondition != NULL))
		first = tree_Node_Next(first);


	if (first && (NODE_POINTER(first)->sleepUntil <= __smp_Now && NODE_POINTER(first)->waitCondition == NULL) && first->key > priority)
		// the current thread is still the first, return
		return 0;

	// we need a real context switch
	// first save the current thread context
	if (t) {
		t->lastTick = __smp_Now;
		memcpy(&t->state, &__smp_IntSwitch_LastState, sizeof(smp_thread_State));
		tree_Node *node = 0;
		bool       ok   = false;
		while (!ok)
			node = tree_InsertNode(__smp_ThreadsWaiting, priority++, &ok);
		NODE_POINTER(node) = t;
	}

	if (!first) {
		// no thread available, load a dummy idle thread
		__smp_IntSwitch_LastState.rip = (uint64_t)__smp_Switch_Idle;
		__smp_IntSwitch_LastState.rflags |= 1 << 9; // Interrupt Enable flag
		__smp_Current[0] = 0;
	} else {
		// load the new context
		memcpy(&__smp_IntSwitch_LastState, &NODE_POINTER(first)->state, sizeof(smp_thread_State));
		__smp_Current[0] = NODE_POINTER(first);
		tree_Delete(__smp_ThreadsWaiting, first);
	}

	return 1;
}
