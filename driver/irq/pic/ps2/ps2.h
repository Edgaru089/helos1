#pragma once

#include "../../../../util/queue.h"

#ifdef __cplusplus
extern "C" {
#endif


#define IRQ_PIC_PS2_MOUSE_SAMPLERATE 60 // the sample rate of the mouse


#define IRQ_PIC_PS2_KEYBOARD 1  // Keyboard IRQ number
#define IRQ_PIC_PS2_MOUSE    12 // Mouse IRQ number

#define IRQ_PIC_PS2_IOPORT     0x0060
#define IRQ_PIC_PS2_STATUSPORT 0x0064
#define IRQ_PIC_PS2_CMDPORT    0x0064

#define IRQ_PIC_PS2_STATUS_OUTPUT_BUFFER           (1 << 0)
#define IRQ_PIC_PS2_STATUS_INPUT_BUFFER            (1 << 1)
#define IRQ_PIC_PS2_STATUS_SYSTEM_FLAG             (1 << 2)
#define IRQ_PIC_PS2_STATUS_INPUT_BUFFER_IS_COMMAND (1 << 3) // (0 = data written to input buffer is data for PS/2 device, 1 = data written to input buffer is data for PS/2 controller command)
#define IRQ_PIC_PS2_STATUS_UNUSED_4                (1 << 4)
#define IRQ_PIC_PS2_STATUS_UNUSED_5                (1 << 5)
#define IRQ_PIC_PS2_STATUS_TIMEOUT_ERROR           (1 << 6)
#define IRQ_PIC_PS2_STATUS_PARITY_ERROR            (1 << 7)

#define IRQ_PIC_PS2_CONFIG_PORT1_INTERRUPT   (1 << 0) // PS/2 port interrupt (1=Enabled, 0=Disabled)
#define IRQ_PIC_PS2_CONFIG_PORT2_INTERRUPT   (1 << 1) // PS/2 port interrupt (1=Enabled, 0=Disabled)
#define IRQ_PIC_PS2_CONFIG_SYSTEM_POST_OK    (1 << 2) // If the system have passed POST
#define IRQ_PIC_PS2_CONFIG_PORT1_CLOCK       (1 << 4) // PS/2 port clock (1=Disabled, 0=Enabled)
#define IRQ_PIC_PS2_CONFIG_PORT2_CLOCK       (1 << 5) // PS/2 port clock (1=Disabled, 0=Enabled)
#define IRQ_PIC_PS2_CONFIG_PORT1_TRANSLATION (1 << 6) // PS/2 keyboard scancode translation (from Set 2 to Set 1)

#define IRQ_PIC_PS2_ACK      0xfa // ACK code for keyboard and mouse (controller cmds have no ack)
#define IRQ_PIC_PS2_RESET_OK 0xaa // the last output byte when a PS/2 device is reset

#define IRQ_PIC_PS2_CMD_READ_CONFIGBYTE  0x20 // Read byte 0 from controller RAM (configuration byte)
#define IRQ_PIC_PS2_CMD_WRITE_CONFIGBYTE 0x60 // Write byte 0 to controller RAM (config byte)
#define IRQ_PIC_PS2_CMD_DISABLE_MOUSE    0xa9 // Disable second PS/2 port (usually mouse)
#define IRQ_PIC_PS2_CMD_ENABLE_MOUSE     0xa8 // Enable second PS/2 port (usually mouse)
#define IRQ_PIC_PS2_CMD_SEND_MOUSE       0xd4 // Send a data byte to the second PS/2 port (usually mouse)

#define IRQ_PIC_PS2_CMD_DEVICE_GETID                   0xf2
#define IRQ_PIC_PS2_CMD_DEVICE_RESET                   0xff
#define IRQ_PIC_PS2_CMD_DEVICE_MOUSE_DISABLE_REPORTING 0xf5
#define IRQ_PIC_PS2_CMD_DEVICE_MOUSE_ENABLE_REPORTING  0xf4

void irq_pic_ps2_Init();

extern bool irq_pic_ps2_HasMouse;    // does the PS/2 controller have 2 channels?
extern bool irq_pic_ps2_Mouse4Bytes; // the mouse has 4-byte data packages instead of 3; mouse wheel enabled


// size in bytes of the Keyboard/Mouse FIFO buffers
#define IRQ_PIC_PS2_QUEUESIZE_KEYBOARD 64
#define IRQ_PIC_PS2_QUEUESIZE_MOUSE    256

// data queue in bytes for the Keyboard and Mouse IRQs
extern queue irq_pic_ps2_QueueKeyboard, irq_pic_ps2_QueueMouse;


#ifdef __cplusplus
}
#endif
