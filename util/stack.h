#pragma once

#include "../main.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
	void *    data; // the data buffer
	uintptr_t size; // size of data buffer
	void *    top;  // top of the stack, the stack grows downward (from data+size to data)
} stack;

// initialize a stack with a existing buffer
void stack_InitBuffered(stack *s, void *buffer, uintptr_t size);

// pushes one byte to the stack, discarding if full
void stack_PushByte(stack *s, const uint8_t b);

// pops one byte from the top of the stack, returning it
uint8_t stack_PopByte(stack *s);

// pushes Size bytes to the stack, none written if there is not space for all the bytes
void stack_Push(stack *s, const void *buffer, uintptr_t size);

// pops Size bytes from the stack, none popped if there are no enough data
// returns the number of bytes popped (either Size or 0)
uintptr_t stack_Pop(stack *s, void *buffer, uintptr_t size);

// return the byte at the top of the stack
uint8_t stack_TopByte(stack *s);

// copy the bytes at the top of the stack
// returns the number of bytes copied
uintptr_t stack_Top(stack *s, void *buffer, uintptr_t size);

// tells if the stack is empty
bool stack_Empty(stack *s);

// returns the number of bytes in the stack
uintptr_t stack_Size(stack *s);

// returns the empty space left in the stack
uintptr_t stack_Space(stack *s);


#ifdef __cplusplus
}
#endif
