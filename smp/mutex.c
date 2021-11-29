
#include "mutex.h"
#include "kthread.h"
#include "internal.h"
#include "../interrupt/interrupt.h"
#include "../memory/memory.h"
#include "../runtime/panic_assert.h"


smp_Mutex *smp_Mutex_Create() {
	smp_Mutex *m = kMalloc(sizeof(smp_Mutex));
	m->locked    = 0;
	m->waiting   = vector_Create(sizeof(__smp_Thread *));
	return m;
}

void smp_Mutex_Destroy(smp_Mutex *mutex) {
	vector_Destroy(mutex->waiting);
	kFree(mutex);
}


void smp_Mutex_Lock(smp_Mutex *mutex) {
	smp_Mutex_LockTimeout(mutex, 0);
}

bool smp_Mutex_LockTimeout(smp_Mutex *mutex, uintptr_t timeout) {
	INTERRUPT_DISABLE;
	if (!mutex->locked) {
		// Lock the mutex
		mutex->locked = 1;
		INTERRUPT_RESTORE;
		return true;
	}

	__smp_Thread *t = __smp_Current[0];

	// Wait for the unlock
	if (!t->wait)
		t->wait = kMalloc(sizeof(__smp_Thread_Waiting));
	t->wait->type    = __smp_Thread_Waiting_Mutex;
	t->wait->object  = mutex;
	t->wait->data    = 0;
	t->wait->timeout = (timeout ? (__smp_Now + timeout) : 0);
	vector_Push(mutex->waiting, &t);

	INTERRUPT_RESTORE;
	smp_thread_Yield();
	return t->wait->data;
}

bool smp_Mutex_TryLock(smp_Mutex *mutex) {
	INTERRUPT_DISABLE;
	if (!mutex->locked) {
		// Lock the mutex
		mutex->locked = 1;
		INTERRUPT_RESTORE;
		return true;
	}
	INTERRUPT_RESTORE;
	return false;
}

void smp_Mutex_Unlock(smp_Mutex *mutex) {
	INTERRUPT_DISABLE;
	assert(mutex->locked && "Attempt to unlock a mutex not held");

	uintptr_t size = vector_Size(mutex->waiting);
	if (size) {
		// Unlock one waiting thread
		__smp_Thread *waiting  = *((__smp_Thread **)vector_At(mutex->waiting, size - 1));
		waiting->wait->object  = NULL;
		waiting->wait->data    = (void *)1;
		waiting->wait->timeout = 0;
		vector_Resize(mutex->waiting, size - 1);
	} else
		mutex->locked = 0;

	INTERRUPT_RESTORE;
}
