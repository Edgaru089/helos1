#pragma once

#ifndef HELOS
#include <assert.h>
#else

#include "../main.h"
#include "stdio.h"
#include "printf.h"


#ifdef __cplusplus
extern "C" {
#endif


#define assert(expr)                                                                                      \
	do {                                                                                                  \
		if (!(expr))                                                                                      \
			Panicf("Assertion failed: (" __FILE__ ":%d[%s]), Expression: %s", __LINE__, __func__, #expr); \
	} while (0)


// defined in assembly
noreturn void __Panic_HaltSystem();


// Panic() aborts the system after printing the message and some other information.
noreturn inline static void Panic(const char *message) {
	io_Printf("Panic: %s\n", message);
	__Panic_HaltSystem();
}

// Panicf() aborts the system after printing the message using vsnprintf.
noreturn inline static void Panicf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int ret = vsnprintf(Buffer, HELOS_BUFFER_SIZE, fmt, args);
	va_end(args);
	io_Printf("Panic: %s\n", Buffer);
	__Panic_HaltSystem();
}


#ifdef __cplusplus
}
#endif

#endif // HELOS
