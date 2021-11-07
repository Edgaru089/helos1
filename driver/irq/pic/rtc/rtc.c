
#include "rtc.h"
#include "internal.h"
#include "../pic.h"
#include "../internal.h"
#include "../../../../main.h"
#include "../../../../runtime/panic_assert.h"
#include "../../../../interrupt/interrupt.h"


bool pic_rtc_Enabled;

void pic_rtc_Init() {
	assert(irq_pic_Enabled && "pic/rtc requires pic to be enabled");

	if (pic_rtc_Enabled)
		return;
	INTERRUPT_DISABLE;

	outb(0x70, 0x8b);         // select register B, and disable NMI
	uint8_t prev = inb(0x71); // read the current value of register B
	outb(0x70, 0x8B);         // set the index again (a read will reset the index to register D)
	outb(0x71, prev | 0x40);  // write the previous value ORed with 0x40. This turns on bit 6 of register B

	irq_pic_IRQHandlerRaw[8] = __pic_rtc_IRQ8;
	irq_pic_Mask(8, false);

	pic_rtc_Enabled = true;
	INTERRUPT_RESTORE;
}

void pic_rtc_SetHandler(void *handler) {
	__pic_rtc_IRQHandler = handler;
}
