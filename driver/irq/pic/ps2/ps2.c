
#include "ps2.h"
#include "../pic.h"
#include "../internal.h"
#include "internal.h"
#include "../../../../interrupt/interrupt.h"
#include "../../../../runtime/panic_assert.h"
#include "../../../../runtime/stdio.h"
#include "../../../../graphics/graphics.h"
#include "../../../../driver/input/source.h"


bool pic_ps2_HasMouse;
bool pic_ps2_Mouse4Bytes; // the mouse has 4-byte data packages instead of 3

queue_Queue pic_ps2_QueueKeyboard, pic_ps2_QueueMouse;
uint8_t     __pic_ps2_QueueBufferK[PIC_PS2_QUEUESIZE_KEYBOARD], __pic_ps2_QueueBufferM[PIC_PS2_QUEUESIZE_MOUSE];

void pic_ps2_Init() {
	assert(irq_pic_Enabled && "pic_ps2_Init() requires PIC to be enabled");

	// init the Keyboard and Mouse queues
	queue_InitBuffered(&pic_ps2_QueueKeyboard, __pic_ps2_QueueBufferK, PIC_PS2_QUEUESIZE_KEYBOARD);
	queue_InitBuffered(&pic_ps2_QueueMouse, __pic_ps2_QueueBufferM, PIC_PS2_QUEUESIZE_MOUSE);

	uint8_t data;
	INTERRUPT_DISABLE;

	irq_pic_IRQHandler[PIC_PS2_KEYBOARD] = pic_ps2_IRQHandlerK;
	irq_pic_Mask(PIC_PS2_KEYBOARD, false);

	// enable second PS/2 port
	io_WriteConsoleASCII("ENABLE_MOUSE... ");
	__ps2_WriteCommand(PIC_PS2_CMD_ENABLE_MOUSE);

	io_WriteConsoleASCII("CONTROLLER_READ_CONFIGBYTE... ");
	__ps2_WriteCommand(PIC_PS2_CMD_READ_CONFIGBYTE);
	uint8_t config = __ps2_ReadData();
	// write controller mode (|= Port1Translation)
	io_WriteConsoleASCII("CONTROLLER_WRITE_CONFIGBYTE... ");
	__ps2_WriteCommandData(PIC_PS2_CMD_WRITE_CONFIGBYTE, config | PIC_PS2_CONFIG_PORT1_TRANSLATION);

	if (config & PIC_PS2_CONFIG_PORT1_CLOCK) { // mouse not present
		pic_ps2_HasMouse = false;
		io_WriteConsoleASCII("PS/2 Controller has no mouse\n");
		INTERRUPT_RESTORE;
		return; // early out
	}

	// initialize the mouse
	// reset mouse
	io_WriteConsoleASCII("DEVICE_RESET... ");
	__ps2_WriteCommandData(PIC_PS2_CMD_SEND_MOUSE, PIC_PS2_CMD_DEVICE_RESET);
	while ((data = __ps2_ReadData()) != PIC_PS2_RESET_OK) {
		io_Printf("%X ", data);
	}
	io_Printf("%X ", data);

	// enable 4-byte mode for mouse, pure magic!
	pic_ps2_Mouse4Bytes = false;
	__ps2_SetMouseRate(200);
	__ps2_SetMouseRate(100);
	__ps2_SetMouseRate(80);

	io_WriteConsoleASCII("SEND_MOUSE(PS2_DEVICE_ID)");
	__ps2_WriteCommandData(PIC_PS2_CMD_SEND_MOUSE, 0xf2); // get device ID
	__ps2_ReadACK();
	uint8_t id = __ps2_ReadData(); // receive device ID
	io_Printf(", MOUSE PS/2 ID=%d\n", id);
	pic_ps2_Mouse4Bytes = (id == 3); // Z-axis is enabled

	irq_pic_IRQHandler[PIC_PS2_MOUSE] = pic_ps2_IRQHandlerM;
	irq_pic_Mask(PIC_PS2_MOUSE, false);

	// set the actual mouse sample rate
	__ps2_SetMouseRate(PIC_PS2_MOUSE_SAMPLERATE);

	// enable mouse reporting
	io_WriteConsoleASCII("MOUSE_ENABLE_REPORTING... ");
	__ps2_WriteCommandData(PIC_PS2_CMD_SEND_MOUSE, PIC_PS2_CMD_DEVICE_MOUSE_ENABLE_REPORTING);
	__ps2_ReadACK(); // receive ACK

	INTERRUPT_RESTORE;
}

SYSV_ABI void pic_ps2_IRQHandlerK() {
	queue_PushByte(&pic_ps2_QueueKeyboard, inb(PIC_PS2_IOPORT));
}

SYSV_ABI void pic_ps2_IRQHandlerM() {
	queue_PushByte(&pic_ps2_QueueMouse, inb(PIC_PS2_IOPORT));

	while (queue_Size(&pic_ps2_QueueMouse) && !(queue_FrontByte(&pic_ps2_QueueMouse) & (1u << 3)))
		queue_PopByte(&pic_ps2_QueueMouse);

	while (queue_Size(&pic_ps2_QueueMouse) >= (pic_ps2_Mouse4Bytes ? 4 : 3)) {
		unsigned int moveX, moveY, state;

		state = queue_PopByte(&pic_ps2_QueueMouse);

		unsigned int d = queue_PopByte(&pic_ps2_QueueMouse);
		moveX          = d - ((state << 4) & 0x100);
		d              = queue_PopByte(&pic_ps2_QueueMouse);
		moveY          = d - ((state << 3) & 0x100);

		input_source_MoveMouse(moveX, -moveY);

		if (pic_ps2_Mouse4Bytes)
			queue_PopByte(&pic_ps2_QueueMouse);
	}
}
