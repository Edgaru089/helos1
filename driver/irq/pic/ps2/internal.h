#pragma once

#include "../../../../main.h"
#include "ps2.h"
#include "../internal.h"

#ifdef __cplusplus
extern "C" {
#endif


SYSV_ABI void pic_ps2_IRQHandlerK(); // keyboard IRQ1
SYSV_ABI void pic_ps2_IRQHandlerM(); // mouse IRQ12

// waits until the Output Buffer Status (bit 0) of the Status Register is set
// and port 0x60 is ready for data read
static inline void __ps2_PollWait_ReadReady() {
	while ((inb(PIC_PS2_STATUSPORT) & PIC_PS2_STATUS_OUTPUT_BUFFER) == 0)
		asm("pause");
}

// waits until the Input Buffer Status (bit 1) of the Status Register is clear
// and ports 0x60/0x64 are ready for data write
static inline void __ps2_PollWait_WriteReady() {
	while ((inb(PIC_PS2_STATUSPORT) & PIC_PS2_STATUS_INPUT_BUFFER) != 0)
		asm("pause");
}

// waits until port 0x60 is ready and reads the byte in it
static inline uint8_t __ps2_ReadData() {
	__ps2_PollWait_ReadReady();
	return inb(PIC_PS2_IOPORT);
}

// waits until port 0x64 is ready for write, OUTB to it.
static inline void __ps2_WriteCommand(uint8_t cmd) {
	__ps2_PollWait_WriteReady();
	outb(PIC_PS2_CMDPORT, cmd);
}

// waits until port 0x64 is ready, OUTB to it, OUTB to 0x60 for data byte.
static inline void __ps2_WriteCommandData(uint8_t cmd, uint8_t data) {
	__ps2_PollWait_WriteReady();
	outb(PIC_PS2_CMDPORT, cmd);
	__ps2_PollWait_WriteReady();
	outb(PIC_PS2_IOPORT, data);
}

static inline void __ps2_ReadACK() {
	while (__ps2_ReadData() != PIC_PS2_ACK) {}
}

// sets the sample rate of the mouse
static inline void __ps2_SetMouseRate(uint8_t rate) {
	__ps2_WriteCommandData(PIC_PS2_CMD_SEND_MOUSE, 0xf3); // command to the mouse
	__ps2_ReadACK();                                      // read the ACK
	__ps2_WriteCommandData(PIC_PS2_CMD_SEND_MOUSE, rate); // send the rate
	__ps2_ReadACK();                                      // read the ACK
}


#ifdef __cplusplus
}
#endif
