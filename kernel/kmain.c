
#include "../main.h"
#include "kmain.h"

#include "../runtime/stdio.h"
#include "../runtime/panic_assert.h"
#include "../memory/memory.h"
#include "../memory/paging_internal.h"
#include "../interrupt/interrupt.h"
#include "../interrupt/handlers.h"
#include "../interrupt/syscall.h"
#include "../driver/irq/pic/pic.h"
#include "../driver/irq/pic/ps2/ps2.h"

#include "../execformat/pe/reloc.h"
void execformat_pe_ReadSystemHeader(execformat_pe_PortableExecutable *pe);

static void tellRIP() {
	uint64_t a, b;
	asm volatile("leaq (%%rip), %0\n\tleaq runtime_InitPaging(%%rip), %1"
				 : "=r"(a), "=r"(b));
	io_Printf("tellRIP(): Stack position: %llx, RIP=%llx, kMain_StackPosition:%llx(%llx), interrupt_Int128: %llx\n", &a, a, (uint64_t)&kMain_StackPosition, b, (uint64_t)interrupt_Int128);
}


SYSV_ABI void kMain() {
	io_WriteConsoleASCII("Yes! kMain survived!\n");

	uint64_t a;
	asm volatile("leaq (%%rip), %0"
				 : "=r"(a));
	io_Printf("Stack position: %llx, RIP=%llx, runtime_InitPaging:%llx, interrupt_Int128: %llx\n", &a, a, (uint64_t)runtime_InitPaging, (uint64_t)interrupt_Int128);

	interrupt_Init();
	io_WriteConsoleASCII("Interrupts initialized\n");

	Syscall(4, 1, 2, 3, 4, 5, 6);
	io_WriteConsoleASCII("Returning from Syscall()\n");

	tellRIP();

	irq_pic_Init();
	io_WriteConsoleASCII("PIC IRQ OK\n");
	irq_pic_ps2_Init();
	io_WriteConsoleASCII("PIC PS/2 OK\n");

	for (;;) {
		asm volatile("hlt");
		io_WriteConsoleASCII("kMain: Interrupt hit\n");
	}
}
