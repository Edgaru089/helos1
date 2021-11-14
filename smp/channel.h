#pragma once

#include "condiction.h"

#include "../main.h"
#include "../util/queue.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


// Channel is a one-way data channel across threads, with a fix-sized buffer.
typedef struct {
	queue_Queue *  queue;       // internal buffer of data
	void *         buffer;      // buffer for at least one piece of data
	vector_Vector *send, *recv; // __smp_Channel_Waiter, for send and receive to wait on, and notify the other
	uintptr_t      objSize;     // size in bytes of the underlying object, 0 if destroyed
	intptr_t       waitcnt;     // (unused) >0: Send is waiting;  <0: Recv is waiting;  ==0: Nobody is waiting
} smp_Channel;

// Create allocates a new Channel.
smp_Channel *smp_Channel_Create(uintptr_t objectSize, uintptr_t bufferSize);

// Destroy frees a Channel.
void smp_Channel_Destroy(smp_Channel *chan);

// Send sends COUNT objects pointed at DATA down the channel.
//
// It waits if there are no one to receive it, and the buffer is already full.
// It will always return COUNT, unless the struct is destroyed.
//
// Returns the number of objects sent.
uintptr_t smp_Channel_Send(smp_Channel *chan, void *data, uintptr_t count);

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


#ifdef __cplusplus
}
#endif
