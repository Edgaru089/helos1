
#include "kthread.h"
#include "kthread_switch.h"
#include "internal.h"

#include "../util/tree.h"
#include "../interrupt/interrupt.h"
#include "../driver/irq/pic/rtc/rtc.h"


smp_thread_ID smp_thread_Init() {
	INTERRUPT_DISABLE;
	__smp_Threads        = tree_Create(sizeof(__smp_Thread));
	__smp_ThreadsWaiting = tree_Create(sizeof(void *));
	__smp_Now            = 1;

	smp_thread_ID id   = 1;
	tree_Node *   node = tree_Insert(__smp_Threads, 1, NULL);
	__smp_Thread *t    = (__smp_Thread *)node->data;
	t->nice            = SMP_NICENESS_DEFAULT;
	t->id              = id;
	t->lastTick        = 1;
	__smp_Current[0]   = t;

	if (!pic_rtc_Enabled)
		pic_rtc_Init();
	pic_rtc_SetHandler(__smp_IntSwitch);

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
}
