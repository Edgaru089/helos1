
#include "pic.h"
#include "internal.h"

#include "../../../runtime/panic_assert.h"
#include "../../../interrupt/interrupt.h"
#include "../../../runtime/stdio.h"

#include <string.h>


irq_pic_IRQHandlerType irq_pic_IRQHandler[16];
bool                   irq_pic_Enabled;

void irq_pic_Init() {
	assert(interrupt_Enabled && "Interrupt must be set up before PIC init");

	INTERRUPT_DISABLE;

	outb_wait(PIC1_DATA, 0xff); // mask all IRQs
	outb_wait(PIC2_DATA, 0xff);

	outb_wait(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	outb_wait(PIC1_DATA, IRQ_PIC_INT_OFFSET_MASTER); // ICW2: Master PIC interrupt vector offset
	outb_wait(PIC1_DATA, 4);                         // ICW3: tell Master PIC that there is a slave PIC at IRQ2
	outb_wait(PIC1_DATA, ICW4_8086);

	outb_wait(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	outb_wait(PIC2_DATA, IRQ_PIC_INT_OFFSET_SLAVE); // ICW2: Slave PIC interrupt vector offset
	outb_wait(PIC2_DATA, 2);                        // ICW3: tell Slave PIC its cascade identity
	outb_wait(PIC2_DATA, ICW4_8086);

	outb_wait(PIC1_DATA, 0xfb); // mask all IRQs except IRQ2 (for slave)
	outb_wait(PIC2_DATA, 0xff);

	memset(irq_pic_IRQHandler, 0, sizeof(irq_pic_IRQHandler)); // reset all IRQ handlers

	// map the IRQ handlers
	interrupt_MapHandler(irq_pic_IntHandler20h, 0x20);
	interrupt_MapHandler(irq_pic_IntHandler21h, 0x21);
	interrupt_MapHandler(irq_pic_IntHandler22h, 0x22);
	interrupt_MapHandler(irq_pic_IntHandler23h, 0x23);
	interrupt_MapHandler(irq_pic_IntHandler24h, 0x24);
	interrupt_MapHandler(irq_pic_IntHandler25h, 0x25);
	interrupt_MapHandler(irq_pic_IntHandler26h, 0x26);
	interrupt_MapHandler(irq_pic_IntHandler27h, 0x27);
	interrupt_MapHandler(irq_pic_IntHandler28h, 0x28);
	interrupt_MapHandler(irq_pic_IntHandler29h, 0x29);
	interrupt_MapHandler(irq_pic_IntHandler2ah, 0x2a);
	interrupt_MapHandler(irq_pic_IntHandler2bh, 0x2b);
	interrupt_MapHandler(irq_pic_IntHandler2ch, 0x2c);
	interrupt_MapHandler(irq_pic_IntHandler2dh, 0x2d);
	interrupt_MapHandler(irq_pic_IntHandler2eh, 0x2e);
	interrupt_MapHandler(irq_pic_IntHandler2fh, 0x2f);


	irq_pic_Enabled = true;
	INTERRUPT_RESTORE;
}

void irq_pic_InitRemap(int offset_master, int offset_slave) {
	uint8_t a1, a2; // existing PIC masks
	a1 = inb(PIC1_DATA);
	a2 = inb(PIC2_DATA);

	outb_wait(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	outb_wait(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	outb_wait(PIC1_DATA, offset_master); // ICW2: Master PIC interrupt vector offset
	outb_wait(PIC2_DATA, offset_slave);  // ICW2: Slave  PIC interrupt vector offset
	outb_wait(PIC1_DATA, 4);             // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb_wait(PIC2_DATA, 2);             // ICW3: tell Slave PIC its cascade identity (0000 0010)
	outb_wait(PIC1_DATA, ICW4_8086);
	outb_wait(PIC2_DATA, ICW4_8086);

	outb_wait(PIC1_DATA, a1); // restore saved masks
	outb_wait(PIC2_DATA, a1);
}

void irq_pic_Mask(uint8_t irq_line, bool masked) {
	uint16_t port;

	if (irq_line < 8)
		port = PIC1_DATA;
	else {
		port = PIC2_DATA;
		irq_line -= 8;
	}

	if (masked)
		outb(port, inb(port) | (1 << irq_line));
	else
		outb(port, inb(port) & ~(1 << irq_line));
}


static inline uint16_t __irq_pic_GetRegister(uint8_t ocw3) {
	/* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
	 * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
	outb_wait(PIC1_COMMAND, ocw3);
	outb_wait(PIC2_COMMAND, ocw3);
	return ((uint16_t)inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

uint16_t irq_pic_GetIRR() {
	return __irq_pic_GetRegister(0x0a); // OCW3 Command: Read IRR
}
uint16_t irq_pic_GetISR() {
	return __irq_pic_GetRegister(0x0b); // OCW3 Command: Read ISR
}
