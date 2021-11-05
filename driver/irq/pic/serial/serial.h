#pragma once

#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif


// Serial port state.
//
// Line protocol defaults to 8N1 (8 bits, no parity, one stop bit)
typedef struct {
	int  port; // IO Port
	bool ok;   // Is the port usable?
} pic_serial_Port;

extern pic_serial_Port pic_serial_COM1, pic_serial_COM2;

// Init initializes a serial port. Returns true if success.
//
// TODO lineFlags is ignored by now, always 8N1
bool pic_serial_Init(pic_serial_Port *port, int baudrate, int lineFlags);

// Write writes a string to a serial port, if the port is usable.
//
// Writes n chars. If n is 0, writes until it reaches NUL.
void pic_serial_Write(pic_serial_Port *port, const char *str, int n);


#ifdef __cplusplus
}
#endif
