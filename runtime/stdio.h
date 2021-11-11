#pragma once

#include "../main.h"

#include "unicode.h"


#ifdef __cplusplus
extern "C" {
#endif

// io_WriteConsole converts UTF-8 string given to Unicode(BMP only),
// then writing it to the Stdout SIMPLE_TEXT_OUTPUT_INTERFACE or the graphical console.
//
// All \n not preceeded by \r is substituted by \r\n. (WIP)
void io_WriteConsole(const char *str);

void io_WriteConsoleASCII(const char *str);

// io_Printf is a printf() replacement, printing to WriteConsole function.
int io_Printf(const char *format, ...);

// io_Error prints error information directly to the screen.
void io_Error(const char *str);
void io_ErrorASCII(const char *str);
int  io_Errorf(const char *format, ...);


// Debugging printing marcos
#ifndef NDEBUG
#define DEBUG(...)                                       \
	do {                                                 \
		printf(__FILE__ ":%d(%s) ", __LINE__, __func__); \
		printf(__VA_ARGS__);                             \
	} while (0)
#else
#define DEBUG(...)
#endif


#ifdef __cplusplus
}
#endif
