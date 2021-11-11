
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

/* A little snippet for testing the handler:
	asm volatile(
		"mov $0, %rax\n\t"
		"mov $1, %rbx\n\t"
		"mov $2, %rcx\n\t"
		"mov $3, %rdx\n\t"
		"mov $4, %rsi\n\t"
		"mov $5, %rdi\n\t"
		"mov $6, %rbp\n\t"
		"mov $8, %r8\n\t"
		"mov $9, %r9\n\t"
		"mov $10, %r10\n\t"
		"mov $11, %r11\n\t"
		"mov $12, %r12\n\t"
		"mov $13, %r13\n\t"
		"mov $14, %r14\n\t"
		"mov $15, %r15\n\t"
		"int3");
 */

SYSV_ABI void interrupt_Handler(int vec, int errcode, uint64_t rip, uint64_t rax, uint64_t rbx, uint64_t rcx, uint64_t rdx, uint64_t rsi, uint64_t rdi, uint64_t rbp, uint64_t rsp, uint64_t r8, uint64_t r9, uint64_t r10, uint64_t r11, uint64_t r12, uint64_t r13, uint64_t r14, uint64_t r15) {
	asm volatile("cli");
	uint64_t cr2;
	asm volatile("mov %%cr2, %0"
				 : "=r"(cr2));
	io_Errorf("Panic: INT %02xh: %s, err=%d(0x%02x), RIP=%llx, CR2=%llx\n"
			  "    RAX=%016llX, RBX=%016llX, RCX=%016llX, RDX=%016llX\n"
			  "    RSI=%016llX, RDI=%016llX, RBP=%016llX, RSP=%016llX\n"
			  "     R8=%016llX,  R9=%016llX, R10=%016llX, R11=%016llX\n"
			  "    R12=%016llX, R13=%016llX, R14=%016llX, R15=%016llX",
			  vec, interrupt_Descriptions[vec], errcode, errcode, rip, cr2, rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp, r8, r9, r10, r11, r12, r13, r14, r15);
	__Panic_HaltSystem();
}

// handler for INT 80h
SYSV_ABI void interrupt_Handler128(int a, int b, int c, int d, int e, int f) {
	int opcode;
	asm volatile("mov %%eax, %0"
				 : "=rm"(opcode)); // read the opcode
	io_Printf("INT 80h: EAX(opcode)=%d, abcdef=[%d,%d,%d,%d,%d,%d]\n", opcode, a, b, c, d, e, f);
}
