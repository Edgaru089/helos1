#pragma once

#include "../../../../util/queue.h"
#include "../../../../smp/condiction.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif


// Default buffer size for serial input
#ifndef PIC_SERIAL_DEFAULT_BUFFERSIZE
#define PIC_SERIAL_DEFAULT_BUFFERSIZE 16
#endif

// Serial port state.
//
// Line protocol defaults to 8N1 (8 bits, no parity, one stop bit)
typedef struct {
	int  port; // IO Port
	int  irq;  // IRQ, 0 for disable
	bool ok;   // Is the port usable?

	queue_Queue    buffer; // input byte buffer
	smp_Condition *cond;   // input wait condiction
} pic_serial_Port;

extern pic_serial_Port pic_serial_COM1, pic_serial_COM2;

// Init initializes a serial port. Returns true if success.
//
// TODO lineFlags is ignored by now, always 8N1
bool pic_serial_Init(pic_serial_Port *port, int baudrate, int lineFlags);

// InitInput initializes input for a serial port. Returns true if success.
bool pic_serial_InitInput(pic_serial_Port *port);

// Write writes a string to a serial port, if the port is usable.
//
// Writes n chars. If n is 0, writes until it reaches NUL.
void pic_serial_Write(pic_serial_Port *port, const char *str, int n);


#ifdef __cplusplus
}
#endif
