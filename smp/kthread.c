
#include "kthread.h"
#include "kthread_switch.h"
#include "internal.h"

#include "../util/tree.h"
#include "../interrupt/interrupt.h"
#include "../driver/irq/pic/rtc/rtc.h"
#include "../memory/memory.h"
#include "../memory/paging_internal.h"


smp_thread_ID smp_thread_Init() {
	INTERRUPT_DISABLE;
	__smp_Threads        = tree_Create(sizeof(__smp_Thread));
	__smp_ThreadsWaiting = tree_Create(sizeof(void *));
	__smp_Now            = 1;

	smp_thread_ID id   = ++__smp_Idallo;
	tree_Node *   node = tree_InsertNode(__smp_Threads, id, NULL);
	__smp_Thread *t    = (__smp_Thread *)node->data;
	t->nice            = SMP_NICENESS_DEFAULT;
	t->id              = id;
	t->lastTick        = 1;
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

	t->state.cs  = GDT_EXEC_SELECTOR;
	t->state.ss  = 0;
	t->state.rip = (uint64_t)entry;

	t->state.rdi = args->a;
	t->state.rsi = args->b;
	t->state.rdx = args->c;
	t->state.rcx = args->d;
	t->state.r8  = args->e;
	t->state.r9  = args->f;

	// allocate a new 4K stack
	uint64_t newstack = memory_AllocateKernelMapping(4096, 4096);
	paging_map_PageAllocated(newstack, 1, MAP_PROT_READ | MAP_PROT_WRITE);
	t->state.rsp = newstack + 4096;

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

void smp_thread_Yield() {
	__smp_PauseTicker = true;
	asm volatile("int $0x28"); // TODO This is just quick and dirty to get into the scheduler
}

void smp_thread_Sleep(int ticks) {
	INTERRUPT_DISABLE;
	__smp_Current[0]->lastTick = __smp_Now + ticks;
	INTERRUPT_RESTORE;
	smp_thread_Yield();
}
