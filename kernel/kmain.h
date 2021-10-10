#pragma once

#include "../main.h"

#ifdef __cplusplus
extern "C" {
#endif

// set the position of the top of stack before calling kMain_Init()
extern uint64_t kMain_StackPosition;
extern char     kMain_StackData[], kMain_StackData_End[];


typedef SYSV_ABI void (*kMainType)();

// written in Assembly, this function deals with stack, registers, etc, and then calls kMain.
//
// remember setting kMain_StackPosition before calling kMain_Init()
SYSV_ABI noreturn void kMain_Init();

// this is the real main function.
// it should only be called by kMain_Init()
SYSV_ABI void kMain();


#ifdef __cplusplus
} // extern "C"
#endif
