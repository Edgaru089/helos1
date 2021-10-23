
#include "stack.h"
#include "../runtime/stdio.h"
#include <memory.h>


void stack_InitBuffered(stack *s, void *buffer, uintptr_t size) {
	s->data = buffer;
	s->size = size;
	s->top  = buffer + size;
}

void stack_PushByte(stack *s, const uint8_t b) {
	if (s->top <= s->data) {
		io_Printf("stack_PushByte: full[%llu bytes], discarding byte 0x%x\n", s->size, b);
		return;
	}
	*(uint8_t *)--s->top = b;
}

uint8_t stack_PopByte(stack *s) {
	if (s->top == s->data + s->size) {
		io_WriteConsoleASCII("stack_PopByte: popping an empty stack\n");
		return 0;
	}
	return *(uint8_t *)s->top++;
}

void stack_Push(stack *s, const void *buffer, uintptr_t size) {
	if (stack_Space(s) < size) {
		io_Printf("stack_Push: insufficient space (%d/%d bytes, want %d bytes)\n", stack_Size(s), s->size, size);
		return;
	}
	s->top -= size;
	memcpy(s->top, buffer, size);
}

uintptr_t stack_Pop(stack *s, void *buffer, uintptr_t size) {
	if (stack_Size(s) < size)
		return 0;
	memcpy(buffer, s->top, size);
	s->top += size;
	return size;
}

uint8_t stack_TopByte(stack *s) {
	if (stack_Empty(s)) {
		io_WriteConsoleASCII("stack_TopByte: called on an empty stack\n");
		return 0;
	}
	return *(uint8_t *)s->top;
}

uintptr_t stack_Top(stack *s, void *buffer, uintptr_t size) {
	if (stack_Size(s) < size)
		return 0;
	memcpy(buffer, s->top, size);
	return size;
}

bool stack_Empty(stack *s) {
	return s->top == s->data + s->size;
}

uintptr_t stack_Size(stack *s) {
	return s->data + s->size - s->top;
}

uintptr_t stack_Space(stack *s) {
	return s->top - s->data;
}

void stack_Clear(stack *s) {
	s->top = s->data + s->size;
}
