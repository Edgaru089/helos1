
#include "ps2.h"
#include "../pic.h"
#include "../internal.h"
#include "internal.h"
#include "../../../../interrupt/interrupt.h"
#include "../../../../runtime/panic_assert.h"
#include "../../../../runtime/stdio.h"
#include "../../../../graphics/graphics.h"
#include "../../../../driver/input/input.h"
#include "../../../../driver/input/source.h"


bool    pic_ps2_HasMouse;
bool    pic_ps2_Mouse4Bytes;    // the mouse has 4-byte data packages instead of 3
uint8_t __pic_ps2_MouseButtons; // 0/0/b5th/b4th/0/bm/br/bl, 7~0 bits

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

static inline bool __pic_ps2_DecodeKeyScancode1(input_Event *event); // too long, kept at the back

SYSV_ABI void pic_ps2_IRQHandlerK() {
	queue_PushByte(&pic_ps2_QueueKeyboard, inb(PIC_PS2_IOPORT));

	input_Event e;
	if (__pic_ps2_DecodeKeyScancode1(&e)) {
		if (e.Type == input_EventType_KeyPressed)
			input_source_PressKey(e.Key.Key);
		else if (e.Type == input_EventType_KeyReleased)
			input_source_ReleaseKey(e.Key.Key);
	}
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

		if (__pic_ps2_MouseButtons != (state & 7)) {
			// Mouse button state change
			// Left
			if (!(__pic_ps2_MouseButtons & 1) && (state & 1))
				input_source_PressMouse(input_MouseButton_Left);
			if ((__pic_ps2_MouseButtons & 1) && !(state & 1))
				input_source_ReleaseMouse(input_MouseButton_Left);
			// Right
			if (!(__pic_ps2_MouseButtons & 2) && (state & 2))
				input_source_PressMouse(input_MouseButton_Right);
			if ((__pic_ps2_MouseButtons & 2) && !(state & 2))
				input_source_ReleaseMouse(input_MouseButton_Right);
			// Middle
			if (!(__pic_ps2_MouseButtons & 4) && (state & 4))
				input_source_PressMouse(input_MouseButton_Middle);
			if ((__pic_ps2_MouseButtons & 4) && !(state & 4))
				input_source_ReleaseMouse(input_MouseButton_Middle);

			__pic_ps2_MouseButtons = (state & 7);
		}
	}
}


const input_Key __pic_ps2_KeyScancode1[] = {input_Key_Unknown, input_Key_Escape, input_Key_Num1, input_Key_Num2, input_Key_Num3, input_Key_Num4, input_Key_Num5, input_Key_Num6, input_Key_Num7, input_Key_Num8, input_Key_Num9, input_Key_Num0, input_Key_Hyphen, input_Key_Equal, input_Key_Backspace, input_Key_Tab, input_Key_Q, input_Key_W, input_Key_E, input_Key_R, input_Key_T, input_Key_Y, input_Key_U, input_Key_I, input_Key_O, input_Key_P, input_Key_LBracket, input_Key_RBracket, input_Key_Enter, input_Key_LControl, input_Key_A, input_Key_S, input_Key_D, input_Key_F, input_Key_G, input_Key_H, input_Key_J, input_Key_K, input_Key_L, input_Key_Semicolon, input_Key_Quote, input_Key_Tilde, input_Key_LShift, input_Key_Backslash, input_Key_Z, input_Key_X, input_Key_C, input_Key_V, input_Key_B, input_Key_N, input_Key_M, input_Key_Comma, input_Key_Period, input_Key_Slash, input_Key_RShift, input_Key_Multiply, input_Key_LAlt, input_Key_Space, input_Key_CapsLock, input_Key_F1, input_Key_F2, input_Key_F3, input_Key_F4, input_Key_F5, input_Key_F6, input_Key_F7, input_Key_F8, input_Key_F9, input_Key_F10, input_Key_NumLock, input_Key_ScrollLock, input_Key_Numpad7, input_Key_Numpad8, input_Key_Numpad9, input_Key_Subtract, input_Key_Numpad4, input_Key_Numpad5, input_Key_Numpad6, input_Key_Add, input_Key_Numpad1, input_Key_Numpad2, input_Key_Numpad3, input_Key_Numpad0, input_Key_Period, input_Key_Unknown, input_Key_Unknown, input_Key_Unknown, input_Key_F11, input_Key_F12};

static inline bool __pic_ps2_DecodeKeyScancode1(input_Event *event) {
	uint8_t byte0 = queue_FrontByte(&pic_ps2_QueueKeyboard);

	if (byte0 == 0xfa) // ACK
		queue_PopByte(&pic_ps2_QueueKeyboard);
	else if (byte0 >= 0x01 && byte0 <= 0x58) {
		queue_PopByte(&pic_ps2_QueueKeyboard);
		event->Type    = input_EventType_KeyPressed;
		event->Key.Key = __pic_ps2_KeyScancode1[byte0];
		return true;
	} else if (byte0 >= 0x81 && byte0 <= 0xd8) {
		queue_PopByte(&pic_ps2_QueueKeyboard);
		event->Type    = input_EventType_KeyReleased;
		event->Key.Key = __pic_ps2_KeyScancode1[byte0 - 0x80];
		return true;
	} else if (byte0 == 0xe0) {
		if (queue_Size(&pic_ps2_QueueKeyboard) < 2)
			return false;

		queue_PopByte(&pic_ps2_QueueKeyboard);
		uint8_t byte1 = queue_PopByte(&pic_ps2_QueueKeyboard);

		event->Type = input_Key_Unknown;
		switch (byte1) {
			case 0x1c:
				event->Type    = input_EventType_KeyPressed;
				event->Key.Key = input_Key_Enter; // Enter on the Numpad
				break;
			case 0x1d:
				event->Type    = input_EventType_KeyPressed;
				event->Key.Key = input_Key_RControl;
				break;
			case 0x35:
				event->Type    = input_EventType_KeyPressed;
				event->Key.Key = input_Key_Divide; // Numpad
				break;
			case 0x38:
				event->Type    = input_EventType_KeyPressed;
				event->Key.Key = input_Key_RAlt;
				break;
			case 0x47:
				event->Type    = input_EventType_KeyPressed;
				event->Key.Key = input_Key_Home;
				break;
			case 0x48:
				event->Type    = input_EventType_KeyPressed;
				event->Key.Key = input_Key_Up;
				break;
			case 0x49:
				event->Type    = input_EventType_KeyPressed;
				event->Key.Key = input_Key_PageUp;
				break;
			case 0x4b:
				event->Type    = input_EventType_KeyPressed;
				event->Key.Key = input_Key_Left;
				break;
			case 0x4d:
				event->Type    = input_EventType_KeyPressed;
				event->Key.Key = input_Key_Right;
				break;
			case 0x4f:
				event->Type    = input_EventType_KeyPressed;
				event->Key.Key = input_Key_End;
				break;
			case 0x50:
				event->Type    = input_EventType_KeyPressed;
				event->Key.Key = input_Key_Down;
				break;
			case 0x51:
				event->Type    = input_EventType_KeyPressed;
				event->Key.Key = input_Key_PageDown;
				break;
			case 0x52:
				event->Type    = input_EventType_KeyPressed;
				event->Key.Key = input_Key_Insert;
				break;
			case 0x53:
				event->Type    = input_EventType_KeyPressed;
				event->Key.Key = input_Key_Delete;
				break;

			case 0x1c + 0x80:
				event->Type    = input_EventType_KeyReleased;
				event->Key.Key = input_Key_Enter;
				break;
			case 0x1d + 0x80:
				event->Type    = input_EventType_KeyReleased;
				event->Key.Key = input_Key_RControl;
				break;
			case 0x35 + 0x80:
				event->Type    = input_EventType_KeyReleased;
				event->Key.Key = input_Key_Divide;
				break;
			case 0x38 + 0x80:
				event->Type    = input_EventType_KeyReleased;
				event->Key.Key = input_Key_RAlt;
				break;
			case 0x47 + 0x80:
				event->Type    = input_EventType_KeyReleased;
				event->Key.Key = input_Key_Home;
				break;
			case 0x48 + 0x80:
				event->Type    = input_EventType_KeyReleased;
				event->Key.Key = input_Key_Up;
				break;
			case 0x49 + 0x80:
				event->Type    = input_EventType_KeyReleased;
				event->Key.Key = input_Key_PageUp;
				break;
			case 0x4b + 0x80:
				event->Type    = input_EventType_KeyReleased;
				event->Key.Key = input_Key_Left;
				break;
			case 0x4d + 0x80:
				event->Type    = input_EventType_KeyReleased;
				event->Key.Key = input_Key_Right;
				break;
			case 0x4f + 0x80:
				event->Type    = input_EventType_KeyReleased;
				event->Key.Key = input_Key_End;
				break;
			case 0x50 + 0x80:
				event->Type    = input_EventType_KeyReleased;
				event->Key.Key = input_Key_Down;
				break;
			case 0x51 + 0x80:
				event->Type    = input_EventType_KeyReleased;
				event->Key.Key = input_Key_PageDown;
				break;
			case 0x52 + 0x80:
				event->Type    = input_EventType_KeyReleased;
				event->Key.Key = input_Key_Insert;
				break;
			case 0x53 + 0x80:
				event->Type    = input_EventType_KeyReleased;
				event->Key.Key = input_Key_Delete;
				break;
		}

		if (event->Key.Key != input_Key_Unknown)
			return true;

	} else if (byte0 == 0xe1) {
		if (queue_Size(&pic_ps2_QueueKeyboard) < 3)
			return false;
		queue_PopByte(&pic_ps2_QueueKeyboard);
		queue_PopByte(&pic_ps2_QueueKeyboard);
		queue_PopByte(&pic_ps2_QueueKeyboard);
	}
	return false;
}
