
#include "pic.h"
#include "internal.h"
#include "../../../runtime/stdio.h"


SYSV_ABI void irq_pic_IntHandler(int irq) {
	if (irq >= 8)
		outb(PIC2_COMMAND, PIC_CMD_EOI);
	outb(PIC1_COMMAND, PIC_CMD_EOI);

	if (irq_pic_IRQHandler[irq] == 0)
		// Text output is EXPENSIVE, do that only on unexpected IRQs
		io_Printf("INT %xh (IRQ %d) (no handler)\n", irq + IRQ_PIC_INT_OFFSET_MASTER, irq);
	else {
		//io_Printf("INT %xh (IRQ %d), handler%llx\n", irq + IRQ_PIC_INT_OFFSET_MASTER, irq, irq_pic_IRQHandler[irq]);
		((irq_pic_IRQHandlerType)irq_pic_IRQHandler[irq])(irq_pic_IRQHandler_Data[irq]);
	}
}
