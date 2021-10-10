
#include "interrupt.h"
#include "../runtime/panic_assert.h"


const char *interrupt_Descriptions[] = {
	"Divide Error Execption",
	"Debug Exception",
	"NMI Interrupt",
	"Breakpoint Exception",
	"Overflow Exception",
	"BOUND Range Exceeded Exception",
	"Invalid Opcode Exception",
	"Device Not Available Exception",
	"Double Fault Exception",
	"Coprocessor Segment Overrun",
	"Invalid TSS Exception",
	"Segment Not Present",
	"Stack Fault Exception",
	"General Protection Exception",
	"Page-Fault Exception",
	"Interrupt 15",
	"x87 FPU Floating-Point Error",
	"Alignment Check Exception",
	"Machine-Check Exception",
	"SIMD Floating-Point Exception",
	"Interrupt 20",
	"Control Protection Exception",
	"Interrupt 22",
	"Interrupt 23",
	"Interrupt 24",
	"Interrupt 25",
	"Interrupt 26",
	"Interrupt 27",
	"Interrupt 28",
	"Interrupt 29",
	"Interrupt 30",
	"Interrupt 31",
};

SYSV_ABI void interrupt_Handler(int vec, int errcode, uint64_t rip, int c, int d, int e) {
	io_Printf("Panic: INT %02xh: %s, err=%d(0x%02x), rip=%llx\n", vec, interrupt_Descriptions[vec], errcode, errcode, rip);
	__Panic_HaltSystem();
}

// handler for INT 80h
SYSV_ABI void interrupt_Handler128(int a, int b, int c, int d, int e, int f) {
	int opcode;
	asm volatile("mov %%eax, %0"
				 : "=rm"(opcode)); // read the opcode
	io_Printf("INT 80h: EAX(opcode)=%d, abcdef=[%d,%d,%d,%d,%d,%d]\n", opcode, a, b, c, d, e, f);
}
