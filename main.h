#pragma once

#include "config.h"

#include <stddef.h>
#include <stdint.h>

#define PROJECT_NAME      "Helos1"
#define PROJECT_NAME_LONG L"Helos1"

#define FASTCALL_ABI __attribute__((ms_abi))   // declares a function as Microsoft x64 ABI. Used in the EFI part of the kernel.
#define SYSV_ABI     __attribute__((sysv_abi)) // declares a function as System V AMD64 ABI. Used in the Kernel space (other than EFI bootstrap code).

#define ALIGNED(n) __attribute__((__aligned__(n)))
#define PACKED     __attribute__((__packed__))

#if (defined __STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define NORETURN _Noreturn
#else
#define NORETURN __attribute__((__noreturn__))
#endif

#define noreturn NORETURN

// Don't (uint64_t) a function symbol directly, use this
#define LOCATE_SYMBOL(var, symbol)             \
	asm volatile("leaq " #symbol "(%%rip), %0" \
				 : "=r"(var))


#define HELOS_BUFFER_SIZE 16384
extern char Buffer[HELOS_BUFFER_SIZE] ALIGNED(4096); // general-purpose buffer, user saved (volatile), not used in interrupt handlers


extern const char link_TextStart[], link_TextEnd[];
extern const char link_DataStart[], link_DataEnd[];
extern const char link_RodataStart[], link_RodataEnd[];
extern const char link_BssStart[], link_BssEnd[];
extern const char link_RelocStart[], link_RelocEnd[];
