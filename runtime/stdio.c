
#include "stdio.h"
#include "unicode.h"
#include "printf.h"
#include "../memory/memory.h"
#include "../graphics/graphics.h"

#include <string.h>
#include <stdarg.h>


// printf wants this
void _putchar(char c) {
	if (!graphics_Framebuffer) {
		UINT16 buf[2] = {c, 0};
		efiStdout->OutputString(efiStdout, buf);
	} else {
		console_WriteChar(&HelosGraphics_Color_White, c);
		if (c == '\n') { // swap buffer on newline
			graphics_SwapBuffer();
		}
	}
}

int     __io_WriteConsole_bufSize = 256;
UINT16  __io_WriteConsole_bufferReal[256];
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

void io_WriteConsole(const char *str) {
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

void io_WriteConsoleASCII(const char *str) {
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

char __io_Printf_buffer[4096];

int io_Printf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int ret = vsnprintf(__io_Printf_buffer, sizeof(__io_Printf_buffer), fmt, args);
	va_end(args);

	io_WriteConsole(__io_Printf_buffer);

	return ret;
}

EFI_INPUT_KEY io_PauseForKeystroke() {
	UINTN         index;
	EFI_INPUT_KEY key;
	efiBootServices->WaitForEvent(1, &efiStdin->WaitForKey, &index);
	efiSystemTable->ConIn->ReadKeyStroke(efiSystemTable->ConIn, &key);

	return key;
}
