
#include "interrupt.h"
#include "handlers.h"
#include "../memory/memory.h"
#include "../runtime/stdio.h"
#include "../runtime/panic_assert.h"
#include "testcode.h"

interrupt_DescriptorTableReference *interrupt_IDTR, *interrupt_GDTR;
bool                                interrupt_Enabled;


/*
SYSV_ABI void interrupt_MapHandler(void *handler, int interrupt) {
	//io_Printf("interrupt_MapHandler: handler %llx, int %d\n", handler, interrupt);
	uint64_t *base = (uint64_t *)(KERNEL_IDT_MAPPING + interrupt * 16ull);
	uint64_t  b    = 0;

	b |= (uint64_t)handler & 0xFFFFull;             // Offset[15:0], 15:0
	b |= (uint64_t)GDT_EXEC_SELECTOR << 16;         // Segment Selector, 31:16
	b |= IDT_TYPE_32_INTERRUPT_GATE;                // Type = 32-bit Interrupt Gate, 44:40
	b |= IDT_RING0;                                 // Ring 0, 46:45
	b |= IDT_PRESENT;                               // Present, 47
	b |= ((uint64_t)handler & 0xFFFF0000ull) << 32; // Offset[31:16], 63:48

	*base       = b;
	*(base + 1) = (uint64_t)handler >> 32;
}
*/

// defined in assembly
SYSV_ABI void interrupt_MapHandler(void *handler, int interrupt);


void interrupt_Init() {
	assert(sizeof(interrupt_DescriptorTableReference) == 10 && "GDTR/IDTR size must be 10 bytes");
	assert(offsetof(interrupt_DescriptorTableReference, base) == 2 && "GDTR/IDTR must be packed");

	assert(KERNEL_IDTR_MAPPING % 4 == 0 && "IDTR not aligned to 4-byte");
	assert(KERNEL_GDTR_MAPPING % 4 == 0 && "GDTR not aligned to 4-byte");


	// allocate GDTR
	io_WriteConsoleASCII("interrupt_Init() calling\n");
	interrupt_GDTR         = (interrupt_DescriptorTableReference *)KERNEL_GDTR_MAPPING;
	interrupt_GDTR->length = 4 * GDT_SIZE_BYTES - 1;
	interrupt_GDTR->base   = (void *)KERNEL_GDT_MAPPING;
	io_WriteConsoleASCII("GDTR Written\n");

	// set the 2 dummy gdts
	uint64_t *gdt = (uint64_t *)KERNEL_GDT_MAPPING;
	gdt[0]        = 0;
	gdt[1]        = GDT_EXEC;
	gdt[2]        = GDT_DATA;
	gdt[3]        = GDT_EXEC_RING3;
	gdt[4]        = GDT_DATA_RING3;
	io_WriteConsoleASCII("GDT Installed\n");

	interrupt_LoadGDT(interrupt_GDTR); // set it!
	io_WriteConsoleASCII("GDT OK\n");

	//interrupt_Testcode();
	io_WriteConsoleASCII("Testcode OK\n");


	// allocate IDTR
	//interrupt_IDTR         = kMalloc(sizeof(interrupt_DescriptorTableReference));
	interrupt_IDTR         = (interrupt_DescriptorTableReference *)KERNEL_IDTR_MAPPING;
	interrupt_IDTR->length = KERNEL_IDT_SIZE - 1;
	interrupt_IDTR->base   = (void *)KERNEL_IDT_MAPPING;
	io_WriteConsoleASCII("IDT Written\n");

	interrupt_MapHandler(interrupt_Int0, 0);
	interrupt_MapHandler(interrupt_Int1, 1);
	interrupt_MapHandler(interrupt_Int2, 2);
	interrupt_MapHandler(interrupt_Int3, 3);
	interrupt_MapHandler(interrupt_Int4, 4);
	interrupt_MapHandler(interrupt_Int5, 5);
	interrupt_MapHandler(interrupt_Int6, 6);
	interrupt_MapHandler(interrupt_Int7, 7);
	interrupt_MapHandler(interrupt_Int8, 8);
	interrupt_MapHandler(interrupt_Int9, 9);
	interrupt_MapHandler(interrupt_Int10, 10);
	interrupt_MapHandler(interrupt_Int11, 11);
	interrupt_MapHandler(interrupt_Int12, 12);
	interrupt_MapHandler(interrupt_Int13, 13);
	interrupt_MapHandler(interrupt_Int14, 14);
	interrupt_MapHandler(interrupt_Int15, 15);
	interrupt_MapHandler(interrupt_Int16, 16);
	interrupt_MapHandler(interrupt_Int17, 17);
	interrupt_MapHandler(interrupt_Int18, 18);
	interrupt_MapHandler(interrupt_Int19, 19);
	interrupt_MapHandler(interrupt_Int20, 20);
	interrupt_MapHandler(interrupt_Int21, 21);
	interrupt_MapHandler(interrupt_Int22, 22);
	interrupt_MapHandler(interrupt_Int23, 23);
	interrupt_MapHandler(interrupt_Int24, 24);
	interrupt_MapHandler(interrupt_Int25, 25);
	interrupt_MapHandler(interrupt_Int26, 26);
	interrupt_MapHandler(interrupt_Int27, 27);
	interrupt_MapHandler(interrupt_Int28, 28);
	interrupt_MapHandler(interrupt_Int29, 29);
	interrupt_MapHandler(interrupt_Int30, 30);
	interrupt_MapHandler(interrupt_Int31, 31);
	interrupt_MapHandler(interrupt_Int128, 128);
	io_WriteConsoleASCII("IDT Installed\n");

	interrupt_LoadIDT(interrupt_IDTR); // set it!


	io_WriteConsoleASCII("IDT OK\n");

	interrupt_Enabled = true;
	asm volatile("sti");

	interrupt_ReloadSegments();
	io_WriteConsoleASCII("Segment Registers Reloaded\n");
}
