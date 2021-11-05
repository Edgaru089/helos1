
#include "serial.h"
#include "../internal.h"
#include "../../../../interrupt/interrupt.h"


pic_serial_Port pic_serial_COM1 = {0x3f8, false}, pic_serial_COM2 = {0x2f8, false};

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
