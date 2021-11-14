
#include "channel.h"
#include "../interrupt/interrupt.h"
#include "../memory/memory.h"
#include "kthread.h"
#include "string.h"


smp_Channel *smp_Channel_Create(uintptr_t objectSize, uintptr_t bufferSize) {
	smp_Channel *c = kMalloc(sizeof(smp_Channel));

	c->objSize = objectSize;
	c->buffer  = kMalloc(objectSize);
	c->send    = smp_Condition_Create();
	c->recv    = smp_Condition_Create();

	if (bufferSize) {
		c->queue = kMalloc(sizeof(queue_Queue));
		queue_InitBuffered(c->queue, kMalloc(objectSize * bufferSize), objectSize * bufferSize);
	} else
		c->queue = NULL;

	return c;
}

// Destroy frees a Channel.
// TODO This is unsafe, dont use while there are someone waiting on it
void smp_Channel_Destroy(smp_Channel *c) {
	c->objSize = 0;

	smp_Condition_NotifyAll(c->send, 0);
	smp_Condition_NotifyAll(c->recv, 0);

	kFree(c->queue->data);
	kFree(c->queue);
	kFree(c->buffer);
	kFree(c);
}


static inline uintptr_t __smp_Channel_SendUnbuffered(smp_Channel *c, void *data, uintptr_t count) {
	uintptr_t sent = 0;
}

uintptr_t smp_Channel_Send(smp_Channel *c, void *data, uintptr_t count) {
	uintptr_t sent = 0;

	int i = 0;

	if (c->queue) {
		// Fill the queue first
		INTERRUPT_DISABLE;
		while (i < count && queue_Space(c->queue))
			queue_Push(c->queue, data + i++ * c->objSize, c->objSize);
		INTERRUPT_RESTORE;

		// Notify the waiting crew
		smp_Condition_NotifyAll(c->recv, 0);
		smp_thread_Yield();
	}

	sent = i;

	// If there are leftovers:
	for (; i < count; i++) {
		smp_Condition_Wait(c->send);
		INTERRUPT_DISABLE;
		memcpy(c->buffer, data + i * c->objSize, c->objSize);
		INTERRUPT_RESTORE;
		smp_Condition_NotifyOne(c->recv, (void *)1);
		smp_thread_Yield();
	}


	return sent;
}

// TrySend attempts sending COUNT objects pointed at DATA down the channel.
//
// It gives up if there are no one to receive the data, and the buffer if already full.
//
// Returns the number of objects sent.
uintptr_t smp_Channel_TrySend(smp_Channel *chan, void *data, uintptr_t count);

// Receive reveices at most MAXCOUNT objects, writing them into the buffer DATA.
//
// It returns at once if there are some data, not necessrily filling the entire buffer.
// It waits if there are no data available, it will not return 0 (unless the struct is destroyed).
//
// Returns the number of objects received.
uintptr_t smp_Channel_Receive(smp_Channel *chan, void *data, uintptr_t maxcount);

// TryReceive attempts receiving at most MAXCOUNT objects.
//
// If there are no data available, it returns 0.
//
// Returns the number of objects received.
uintptr_t smp_Channel_TryReceive(smp_Channel *chan, void *data, uintptr_t maxcount);
