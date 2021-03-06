
#include "stdio.h"
#include "unicode.h"
#include "printf.h"
#include "../memory/memory.h"
#include "../graphics/graphics.h"
#include "../driver/irq/pic/serial/serial.h"
#include "../interrupt/interrupt.h"

#include "../efimain.h"
#include <string.h>
#include <stdarg.h>


// printf wants this
void _putchar(char c) {
	pic_serial_Write(&pic_serial_COM1, &c, 1);

#ifndef HELOS_RUNTIME_QUIET
	if (!graphics_Framebuffer) {
		UINT16 buf[2] = {c, 0};
		efiStdout->OutputString(efiStdout, buf);
	} else {
		console_WriteChar(&HelosGraphics_Color_White, c);
		if (c == '\n') { // swap buffer on newline
			graphics_SwapBuffer();
		}
	}
#endif
}

int     __io_WriteConsole_bufSize = 512;
UINT16  __io_WriteConsole_bufferReal[512];
UINT16 *__io_WriteConsole_buffer = __io_WriteConsole_bufferReal;

void __io_WriteConsole_ResizeBuffer(int size) {
	if (__io_WriteConsole_bufSize < size) {
		while (__io_WriteConsole_bufSize < size)
			__io_WriteConsole_bufSize *= 2;

		DEBUG("allocate -> %d", __io_WriteConsole_bufSize);

		if (__io_WriteConsole_buffer != __io_WriteConsole_bufferReal)
			kFree(__io_WriteConsole_buffer);
		__io_WriteConsole_buffer = kMalloc(size * sizeof(UINT16));
	}
}

static inline void __io_WriteConsoleUTF8(const char *str) {
	int size = 0;           // don't include the \0 at the end here
	int len  = strlen(str); // left the \0 out here too

	for (int i = 0;
		 i < len;
		 i += utf8_Decode(str + i, len - i, NULL), size++) {}
	__io_WriteConsole_ResizeBuffer(size + 1);

	uint32_t codepoint;
	for (int i = 0, j = 0;
		 i < len;) {
		i += utf8_Decode(str + i, len - i, &codepoint);
		__io_WriteConsole_buffer[j++] = codepoint;
	}
	__io_WriteConsole_buffer[size] = 0;

	if (!graphics_Framebuffer) {
		efiStdout->OutputString(efiStdout, __io_WriteConsole_buffer);
	} else {
		console_WriteUTF16(&HelosGraphics_Color_White, __io_WriteConsole_buffer, 0);
	}
}

static inline void __io_WriteConsoleASCII(const char *str) {
	if (!graphics_Framebuffer) {
		int len = strlen(str);
		__io_WriteConsole_ResizeBuffer(len + 1);
		for (int i = 0; i <= len; i++)
			__io_WriteConsole_buffer[i] = str[i];
		efiStdout->OutputString(efiStdout, __io_WriteConsole_buffer);
	} else {
		console_WriteASCII(&HelosGraphics_Color_White, str, 0);
	}
}

void io_WriteConsole(const char *str) {
	pic_serial_Write(&pic_serial_COM1, str, 0);
#ifndef HELOS_RUNTIME_QUIET
	__io_WriteConsoleUTF8(str);
#endif
}

void io_WriteConsoleASCII(const char *str) {
	pic_serial_Write(&pic_serial_COM1, str, 0);
#ifndef HELOS_RUNTIME_QUIET
	__io_WriteConsoleASCII(str);
#endif
}

char __io_Printf_buffer[4096];

int io_Printf(const char *fmt, ...) {
	INTERRUPT_DISABLE;
	va_list args;
	va_start(args, fmt);
	int ret = vsnprintf(__io_Printf_buffer, sizeof(__io_Printf_buffer), fmt, args);
	va_end(args);

	io_WriteConsole(__io_Printf_buffer);

	INTERRUPT_RESTORE;
	return ret;
}

void io_Error(const char *str) {
	pic_serial_Write(&pic_serial_COM1, str, 0);
	__io_WriteConsoleUTF8(str);
}

void io_ErrorASCII(const char *str) {
	pic_serial_Write(&pic_serial_COM1, str, 0);
	__io_WriteConsoleASCII(str);
}

int io_Errorf(const char *fmt, ...) {
	INTERRUPT_DISABLE;
	va_list args;
	va_start(args, fmt);
	int ret = vsnprintf(__io_Printf_buffer, sizeof(__io_Printf_buffer), fmt, args);
	va_end(args);

	io_Error(__io_Printf_buffer);

	INTERRUPT_RESTORE;
	return ret;
}
