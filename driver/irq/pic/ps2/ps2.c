
#include "ps2.h"
#include "../pic.h"
#include "../internal.h"
#include "internal.h"
#include "../../../../interrupt/interrupt.h"
#include "../../../../runtime/panic_assert.h"
#include "../../../../runtime/stdio.h"
#include "../../../../graphics/graphics.h"


bool irq_pic_ps2_HasMouse;
bool irq_pic_ps2_Mouse4Bytes; // the mouse has 4-byte data packages instead of 3

queue   irq_pic_ps2_QueueKeyboard, irq_pic_ps2_QueueMouse;
uint8_t __irq_pic_ps2_QueueBufferK[IRQ_PIC_PS2_QUEUESIZE_KEYBOARD], __irq_pic_ps2_QueueBufferM[IRQ_PIC_PS2_QUEUESIZE_MOUSE];

void irq_pic_ps2_Init() {
	assert(irq_pic_Enabled && "irq_pic_ps2_Init() requires PIC to be enabled");

	// init the Keyboard and Mouse queues
	queue_InitBuffered(&irq_pic_ps2_QueueKeyboard, __irq_pic_ps2_QueueBufferK, IRQ_PIC_PS2_QUEUESIZE_KEYBOARD);
	queue_InitBuffered(&irq_pic_ps2_QueueMouse, __irq_pic_ps2_QueueBufferM, IRQ_PIC_PS2_QUEUESIZE_MOUSE);

	uint8_t data;
	INTERRUPT_DISABLE;

	irq_pic_IRQHandler[IRQ_PIC_PS2_KEYBOARD] = irq_pic_ps2_IRQHandlerK;
	irq_pic_Mask(IRQ_PIC_PS2_KEYBOARD, false);

	// enable second PS/2 port
	io_WriteConsoleASCII("ENABLE_MOUSE... ");
	__ps2_WriteCommand(IRQ_PIC_PS2_CMD_ENABLE_MOUSE);

	io_WriteConsoleASCII("CONTROLLER_READ_CONFIGBYTE... ");
	__ps2_WriteCommand(IRQ_PIC_PS2_CMD_READ_CONFIGBYTE);
	uint8_t config = __ps2_ReadData();
	// write controller mode (|= Port1Translation)
	io_WriteConsoleASCII("CONTROLLER_WRITE_CONFIGBYTE... ");
	__ps2_WriteCommandData(IRQ_PIC_PS2_CMD_WRITE_CONFIGBYTE, config | IRQ_PIC_PS2_CONFIG_PORT1_TRANSLATION);

	if (config & IRQ_PIC_PS2_CONFIG_PORT1_CLOCK) { // mouse not present
		irq_pic_ps2_HasMouse = false;
		io_WriteConsoleASCII("PS/2 Controller has no mouse\n");
		INTERRUPT_RESTORE;
		return; // early out
	}

	// initialize the mouse
	// reset mouse
	io_WriteConsoleASCII("DEVICE_RESET... ");
	__ps2_WriteCommandData(IRQ_PIC_PS2_CMD_SEND_MOUSE, IRQ_PIC_PS2_CMD_DEVICE_RESET);
	while ((data = __ps2_ReadData()) != IRQ_PIC_PS2_RESET_OK) {
		io_Printf("%X ", data);
	}
	io_Printf("%X ", data);

	// enable mouse reporting
	io_WriteConsoleASCII("MOUSE_ENABLE_REPORTING... ");
	__ps2_WriteCommandData(IRQ_PIC_PS2_CMD_SEND_MOUSE, IRQ_PIC_PS2_CMD_DEVICE_MOUSE_ENABLE_REPORTING);
	__ps2_ReadACK(); // receive ACK

	// enable 4-byte mode for mouse, pure magic!
	irq_pic_ps2_Mouse4Bytes = false;
	__ps2_SetMouseRate(200);
	__ps2_SetMouseRate(100);
	__ps2_SetMouseRate(80);

	io_WriteConsoleASCII("SEND_MOUSE(PS2_DEVICE_ID)");
	__ps2_WriteCommandData(IRQ_PIC_PS2_CMD_SEND_MOUSE, 0xf2); // get device ID
	__ps2_ReadACK();
	uint8_t id = __ps2_ReadData(); // receive device ID
	io_Printf(", MOUSE PS/2 ID=%d\n", id);
	irq_pic_ps2_Mouse4Bytes = (id == 3); // Z-axis is enabled

	irq_pic_IRQHandler[IRQ_PIC_PS2_MOUSE] = irq_pic_ps2_IRQHandlerM;
	irq_pic_Mask(IRQ_PIC_PS2_MOUSE, false);

	// set the actual mouse sample rate
	__ps2_SetMouseRate(IRQ_PIC_PS2_MOUSE_SAMPLERATE);

	INTERRUPT_RESTORE;
}

SYSV_ABI void irq_pic_ps2_IRQHandlerK() {
	queue_PushByte(&irq_pic_ps2_QueueKeyboard, inb(IRQ_PIC_PS2_IOPORT));
}

SYSV_ABI void irq_pic_ps2_IRQHandlerM() {
	queue_PushByte(&irq_pic_ps2_QueueMouse, inb(IRQ_PIC_PS2_IOPORT));
}
