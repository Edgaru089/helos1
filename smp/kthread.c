
#include "kthread.h"
#include "kthread_switch.h"
#include "internal.h"

#include "../runtime/panic_assert.h"
#include "../util/tree.h"
#include "../interrupt/interrupt.h"
#include "../driver/irq/pic/rtc/rtc.h"
#include "../memory/memory.h"
#include "../memory/paging_internal.h"


// defined in assembly
SYSV_ABI void __smp_thread_EntryPoint();

// for __smp_thread_EntryPoint cleanups
// assumes interrupt is disabled
SYSV_ABI NORETURN void __smp_thread_Cleanup() {
	__smp_Thread *t = __smp_Current[0];
	tree_Delete(__smp_Threads, tree_FindNode(__smp_Threads, t->id));
	__smp_Current[0] = 0;

	// Push the freed stack frame into the pool
	tree_Insert(__smp_StackPool, t->stackframe, NULL);

	// Now that we're pretending to be a idle thread, this should not return
	// thread_Switch() sets the interrupt flag on idle for us
	smp_thread_Yield();
	Panic("__smp_thread_Cleanup: Yield not switching to another thread");
}


smp_thread_ID smp_thread_Init() {
	INTERRUPT_DISABLE;
	__smp_Threads        = tree_Create(sizeof(__smp_Thread));
	__smp_ThreadsWaiting = tree_Create(sizeof(void *));
	__smp_StackPool      = tree_Create(0);
	__smp_Now            = 1;

	smp_thread_ID id   = ++__smp_Idallo;
	tree_Node *   node = tree_InsertNode(__smp_Threads, id, NULL);
	__smp_Thread *t    = (__smp_Thread *)node->data;
	t->nice            = SMP_NICENESS_DEFAULT;
	t->id              = id;
	t->lastTick        = 1;
	t->sleepUntil      = 0;
	t->waitCondition   = NULL;
	__smp_Count        = 1;

	__smp_Current    = kMalloc(sizeof(void *) * __smp_Count);
	__smp_Current[0] = t;

	if (!pic_rtc_Enabled)
		pic_rtc_Init();
	pic_rtc_SetHandler(__smp_IntSwitch);

	INTERRUPT_RESTORE;
	return id;
}

smp_thread_ID smp_thread_Start(void *entry, const smp_thread_Arguments *args, unsigned int nice) {
	INTERRUPT_DISABLE;

	smp_thread_ID id   = ++__smp_Idallo;
	tree_Node *   node = tree_InsertNode(__smp_Threads, id, NULL);
	__smp_Thread *t    = (__smp_Thread *)node->data;
	t->nice            = nice;
	t->id              = id;
	t->lastTick        = __smp_Now;
	t->sleepUntil      = 0;
	t->waitCondition   = NULL;

	t->state.cs  = GDT_EXEC_SELECTOR;
	t->state.ss  = 0;
	t->state.rip = (uint64_t)__smp_thread_EntryPoint;

	t->state.rax = (uint64_t)entry;
	t->state.rdi = args->a;
	t->state.rsi = args->b;
	t->state.rdx = args->c;
	t->state.rcx = args->d;
	t->state.r8  = args->e;
	t->state.r9  = args->f;

	tree_Node *spare = tree_FirstNode(__smp_StackPool);
	if (spare) {
		// use the existing stack from the pool
		t->state.rsp = t->stackframe = spare->key + 4096;
		tree_Delete(__smp_StackPool, spare);
	} else {
		// allocate a new 4K stack
		uint64_t newstack = memory_AllocateKernelMapping(4096, 4096);
		paging_map_PageAllocated(newstack, 1, MAP_PROT_READ | MAP_PROT_WRITE);
		t->state.rsp = t->stackframe = newstack + 4096;
	}

	// insert the thread into the waiting queue
	*((void **)tree_Insert(__smp_ThreadsWaiting, t->lastTick + t->nice, 0)) = t;

	INTERRUPT_RESTORE;
	return id;
}

int smp_thread_Nice(smp_thread_ID id, int newnice) {
	INTERRUPT_DISABLE;
	__smp_Thread *t = tree_Find(__smp_Threads, id);
	if (!t) {
		INTERRUPT_RESTORE;
		return -1;
	} else if (newnice < 0) {
		INTERRUPT_RESTORE;
		return t->nice;
	}

	int oldnice = t->nice;
	t->nice     = newnice;
	INTERRUPT_RESTORE;
	return oldnice;
}

void smp_thread_Sleep(int ticks) {
	INTERRUPT_DISABLE;
	__smp_Current[0]->sleepUntil = __smp_Now + ticks;
	INTERRUPT_RESTORE;
	smp_thread_Yield();
}
