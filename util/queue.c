
#include "queue.h"
#include "../runtime/stdio.h"


void queue_InitBuffered(queue *q, void *buffer, uintptr_t size) {
	q->data = q->begin = q->end = buffer;
	q->size                     = size;
	q->count                    = 0;
}

void queue_PushByte(queue *q, const uint8_t b) {
	if (q->count == q->size) { // no more space
		io_Printf("queue_PushByte: full[%llu bytes], discarding byte 0x%x\n", q->size, b);
		return;
	}

	q->count++;
	*((uint8_t *)(q->end++)) = b;
	if (q->end == q->data + q->size)
		q->end = q->data; // out of the buffer: wrap around
}

uint8_t queue_PopByte(queue *q) {
	if (q->count == 0) {
		io_WriteConsoleASCII("queue_PopByte: poping an empty queue\n");
		return 0;
	}

	q->count--;
	uint8_t data = *((uint8_t *)(q->begin++));
	if (q->begin == q->data + q->size)
		q->begin = q->data; // wrap around
	return data;
}

uint8_t queue_FrontByte(queue *q) {
	if (q->count == 0) {
		io_WriteConsoleASCII("queue_TopByte: accessing an empty queue\n");
		return 0;
	}
	return *((uint8_t *)q->begin);
}

bool queue_Empty(queue *q) {
	return q->count == 0;
}

uintptr_t queue_Size(queue *q) {
	return q->count;
}

uintptr_t queue_Space(queue *q) {
	return q->size - q->count;
}
