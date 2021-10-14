#pragma once

#include "../main.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif


#define GDT_SIZE_BYTES 4
#define GDT_EXEC       0x00AF9A000000FFFFull // Base=0, Limit=max, Access=Present|Ring0|TypeUser|Exec|Readable, Flag=GranularityPage|Long
#define GDT_DATA       0x00EF92000000FFFFull // Base=0, Limit=max, Access=Present|Ring0|TypeUser|Writable, Flag=GranularityPage|Size
#define GDT_EXEC_RING3 0x00AFFA000000FFFFull // Base=0, Limit=max, Access=Present|Ring3|TypeUser|Exec|Readable, Flag=GranularityPage|Long
#define GDT_DATA_RING3 0x00EFF2000000FFFFull // Base=0, Limit=max, Access=Present|Ring3|TypeUser|Writable, Flag=GranularityPage|Size

#define GDT_EXEC_SELECTOR       0x08 // SelectorIndex=1, TableIndicator=GDT(0), Privilege=Ring0
#define GDT_DATA_SELECTOR       0x10 // SelectorIndex=2, TableIndicator=GDT(0), Privilege=Ring0
#define GDT_EXEC_RING3_SELECTOR 0x1B // SelectorIndex=3, TableIndicator=GDT(0), Privilege=Ring3
#define GDT_DATA_RING3_SELECTOR 0x23 // SelectorIndex=4, TableIndicator=GDT(0), Privilege=Ring3

#define IDT_PRESENT (1ull << 47)
#define IDT_RING0   0
#define IDT_RING1   (1ull << 45)
#define IDT_RING2   (2ull << 45)
#define IDT_RING3   (3ull << 45)

#define IDT_TYPE_32_CALL_GATE      (0x0Cull << 40)
#define IDT_TYPE_32_INTERRUPT_GATE (0x0Eull << 40)
#define IDT_TYPE_32_TRAP_GATE      (0x0Full << 40)

typedef struct {
	uint16_t length;
	void *   base;
} PACKED interrupt_DescriptorTableReference;
// address of IDTR and GDTR, allocated by kMalloc() and is never freed
extern interrupt_DescriptorTableReference *interrupt_IDTR, *interrupt_GDTR;

// true if Init() has been called and interrupt handling is on
extern bool interrupt_Enabled;

// initializes interrupt handling like IDT and a dummy GDT
void interrupt_Init();

SYSV_ABI void interrupt_MapHandler(void *handler, int interrupt);


// errorcode is 0 if nonexistent
//
// for IRQs, params are documented in assembly
SYSV_ABI void interrupt_Handler(int vec, int errcode, uint64_t rip, uint64_t rax, uint64_t rbx, uint64_t rcx, uint64_t rdx, uint64_t rsi, uint64_t rdi, uint64_t rbp, uint64_t rsp, uint64_t r8, uint64_t r9, uint64_t r10, uint64_t r11, uint64_t r12, uint64_t r13, uint64_t r14, uint64_t r15);

// defined in assembly
SYSV_ABI void interrupt_ReloadSegments();

SYSV_ABI void interrupt_LoadGDT(uint16_t length_sub1, void *base_ptr);
SYSV_ABI void interrupt_LoadIDT(uint16_t length_sub1, void *base_ptr);


#define INTERRUPT_DISABLE                  \
	uintptr_t __interrupt_flags;           \
	asm volatile("pushf\n\tcli\n\tpop %0"  \
				 : "=r"(__interrupt_flags) \
				 :                         \
				 : "memory")
#define INTERRUPT_RESTORE                  \
	asm volatile("push %0\n\tpopf"         \
				 :                         \
				 : "rm"(__interrupt_flags) \
				 : "memory", "cc")


#ifdef __cplusplus
}
#endif
