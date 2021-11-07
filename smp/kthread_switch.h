#pragma once

#include "../main.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


// State describes the stack, IP and general registers of a State.
typedef struct {
	uint64_t rax, rbx, rcx, rdx;
	uint64_t rsi, rdi, rbp, rsp;
	uint64_t r8, r9, r10, r11;
	uint64_t r12, r13, r14, r15;

	uint64_t rflags;
	uint64_t rip;
	uint16_t ss, cs; // Stack and Code Segments
} PACKED smp_thread_State;

// Last program state saved/restored by IntSwitch()
extern smp_thread_State __smp_IntSwitch_LastState;

// Interrupt handler, defined in assembly
void __smp_IntSwitch();

// Copy the current thread from LastState,
// invoke the scheduler to find a new thread,
// copy the new thread to LastState,
// and returns back to smp_IntSwitch, which restores the new state and IRET.
//
// Returns nonzero if a context switch is in fact required.
SYSV_ABI uintptr_t __smp_Switch();


#ifdef __cplusplus
}
#endif
