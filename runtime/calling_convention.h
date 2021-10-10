
// x64sysvcall int sysv_x64fastcall(void* addr, int numArgs, long args1, args2, args3, args4)
//
// Calls Microsoft x64 ABI functions under System V AMD64 ABI.
//
// This function can handle ONLY up to FOUR arguments.
// numArgs is in fact unused.
//
// Input:   (void* rdi, int rsi, long rdx, rcx, r8, r9)
// Output:   int rax
// Clobbers: flags
__attribute__((sysv_abi)) long sysv_x64fastcall(void *addr, int numArgs, long args1, long args2, long args3, long args4);
