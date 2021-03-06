
#include "condiction.h"
#include "kthread.h"
#include "internal.h"
#include "../interrupt/interrupt.h"
#include "../memory/memory.h"
#include "../runtime/stdio.h"


smp_Condition *smp_Condition_Create() {
	smp_Condition *c = kMalloc(sizeof(smp_Condition));
	c->threads       = vector_Create(sizeof(__smp_Thread *));
	return c;
}

void smp_Condition_Destroy(smp_Condition *c) {
	smp_Condition_NotifyAll(c, (void *)UINTPTR_MAX);
	vector_Destroy(c->threads);
	kFree(c);
}

void *smp_Condition_Wait(smp_Condition *c) {
	INTERRUPT_DISABLE;

	__smp_Thread *t = __smp_Current[0];

	if (!t->wait)
		t->wait = kMalloc(sizeof(__smp_Thread_Waiting));
	t->wait->type    = __smp_Thread_Waiting_Condiction;
	t->wait->object  = c;
	t->wait->data    = 0;
	t->wait->timeout = 0;

	vector_Push(c->threads, &t);

	INTERRUPT_RESTORE;
	smp_thread_Yield();
	return t->wait->data;
}

bool smp_Condition_NotifyOne(smp_Condition *c, void *data) {
	INTERRUPT_DISABLE;

	uintptr_t size = vector_Size(c->threads);
	if (size == 0) {
		INTERRUPT_RESTORE;
		return false;
	}

	__smp_Thread *last = *(__smp_Thread **)vector_At(c->threads, size - 1);
	last->wait->object = NULL;
	last->wait->data   = data;
	vector_Resize(c->threads, size - 1);

	INTERRUPT_RESTORE;
	return true;
}

int smp_Condition_NotifyAll(smp_Condition *c, void *data) {
	INTERRUPT_DISABLE;

	uintptr_t size = vector_Size(c->threads);
	if (size == 0) {
		INTERRUPT_RESTORE;
		return size;
	}

	for (uintptr_t i = 0; i < size; i++) {
		__smp_Thread *t = *(__smp_Thread **)vector_At(c->threads, i);
		t->wait->object = NULL;
		t->wait->data   = data;
	}
	vector_Clear(c->threads);

	INTERRUPT_RESTORE;
	return true;
}
