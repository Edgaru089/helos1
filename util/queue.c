
#include "queue.h"
#include "../runtime/stdio.h"


void queue_InitBuffered(queue_Queue *q, void *buffer, uintptr_t size) {
	q->data = q->begin = q->end = buffer;
	q->size                     = size;
	q->count                    = 0;
}

void queue_PushByte(queue_Queue *q, const uint8_t b) {
	if (q->count == q->size) { // no more space
		io_Errorf("queue_PushByte: full[%llu bytes], discarding byte 0x%x\n", q->size, b);
		return;
	}

	q->count++;
	*((uint8_t *)(q->end++)) = b;
	if (q->end == q->data + q->size)
		q->end = q->data; // out of the buffer: wrap around
}

uint8_t queue_PopByte(queue_Queue *q) {
	if (q->count == 0) {
		io_Error("queue_PopByte: poping an empty queue\n");
		return 0;
	}

	q->count--;
	uint8_t data = *((uint8_t *)(q->begin++));
	if (q->begin == q->data + q->size)
		q->begin = q->data; // wrap around
	return data;
}

void queue_Push(queue_Queue *q, const void *buffer, uintptr_t size) {
	// TODO Optimize queue_Push and queue_Pop
	if (queue_Space(q) < size)
		return;
	for (const uint8_t *i = buffer; i < (const uint8_t *)buffer + size; i++)
		queue_PushByte(q, *i);
}

uintptr_t queue_Pop(queue_Queue *q, void *buffer, uintptr_t size) {
	if (queue_Size(q) < size)
		return 0;
	for (uint8_t *i = buffer; i < (uint8_t *)buffer + size; i++)
		*i = queue_PopByte(q);
	return size;
}

uint8_t queue_FrontByte(queue_Queue *q) {
	if (q->count == 0) {
		io_Error("queue_TopByte: accessing an empty queue\n");
		return 0;
	}
	return *((uint8_t *)q->begin);
}

bool queue_Empty(queue_Queue *q) {
	return q->count == 0;
}

uintptr_t queue_Size(queue_Queue *q) {
	return q->count;
}

uintptr_t queue_Space(queue_Queue *q) {
	return q->size - q->count;
}
