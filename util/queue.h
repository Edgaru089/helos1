#pragma once

#include "../main.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
	void *    data;        // the data buffer
	uintptr_t size;        // size of data buffer
	void *    begin, *end; // begin and past-the-end for in-queue data
	uintptr_t count;       // number of in-queue bytes
} queue;

// initialize a queue with a existing buffer
void queue_InitBuffered(queue *q, void *buffer, uintptr_t size);

// writes one byte to the queue, discarding if full
void queue_PushByte(queue *q, const uint8_t b);

// pops one byte from the front of the queue, returning it
uint8_t queue_PopByte(queue *q);

// return the byte at the front of the queue
uint8_t queue_FrontByte(queue *q);

// tells if the queue is empty
bool queue_Empty(queue *q);

// returns the number of bytes in the queue
uintptr_t queue_Size(queue *q);

// returns the empty space left at the end of the queue
uintptr_t queue_Space(queue *q);


#ifdef __cplusplus
}
#endif
