
#include "serial.h"
#include "../internal.h"
#include "../../../../interrupt/interrupt.h"
#include "../../../../memory/memory.h"
#include "../pic.h"


pic_serial_Port pic_serial_COM1 = {0x3f8, 4, false}, pic_serial_COM2 = {0x2f8, 3, false};

SYSV_ABI void __pic_serial_IRQHandler(pic_serial_Port *port) {
	while ((inb(port->port + 5) & 1)) {
		uint8_t b = inb(port->port);
		queue_PushByte(&port->buffer, b);
		smp_Condition_NotifyAll(port->cond, 0);
	}
}

bool pic_serial_Init(pic_serial_Port *port, int baudrate, int lineFlags) {
	if (115200 % baudrate != 0)
		return false;

	uint16_t divisor = 115200 / baudrate;
	INTERRUPT_DISABLE;

	outb(port->port + 3, 0x80);           // Enable DLAB (set baud rate divisor)
	outb(port->port + 0, divisor & 0xFF); // Set divisor (lo byte)
	outb(port->port + 1, divisor >> 8);   //             (hi byte)
	outb(port->port + 3, 0x03);           // 8 bits, no parity, one stop bit
	outb(port->port + 2, 0xC7);           // Enable FIFO, clear them, with 14-byte threshold
	outb(port->port + 4, 0x0B);           // IRQs enabled, RTS/DSR set
	outb(port->port + 4, 0x1E);           // Set in loopback mode, test the serial chip
	outb(port->port + 0, 0xAE);           // Test serial chip (send byte 0xAE and check if serial returns same byte)

	// Check if serial is faulty (i.e: not same byte as sent)
	if (inb(port->port + 0) != 0xAE) {
		INTERRUPT_RESTORE;
		return false;
	}

	// If serial is not faulty set it in normal operation mode
	// (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
	outb(port->port + 4, 0x0F);
	port->ok = true;

	INTERRUPT_RESTORE;
	return true;
}

bool pic_serial_InitInput(pic_serial_Port *port) {
	// Set IRQ handler for serial input
	if (port->irq) {
		queue_InitBuffered(&port->buffer, kMalloc(PIC_SERIAL_DEFAULT_BUFFERSIZE), PIC_SERIAL_DEFAULT_BUFFERSIZE);
		port->cond = smp_Condition_Create();

		irq_pic_IRQHandler[port->irq]      = __pic_serial_IRQHandler;
		irq_pic_IRQHandler_Data[port->irq] = (uintptr_t)port;
		irq_pic_Mask(port->irq, false);
		outb(port->port + 1, 1);

		return true;
	} else
		return false;
}

void pic_serial_Write(pic_serial_Port *port, const char *str, int n) {
	if (port && port->ok) {
		INTERRUPT_DISABLE;
		if (n != 0) {
			for (int i = 0; i < n; i++) {
				while ((inb(port->port + 5) & 0x20) == 0) {}
				outb(port->port, str[i]);
			}
		} else {
			while (*str != 0) {
				while ((inb(port->port + 5) & 0x20) == 0) {}
				outb(port->port, *str);
				str++;
			}
		}
		INTERRUPT_RESTORE;
	}
}
