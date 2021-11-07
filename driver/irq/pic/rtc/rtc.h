#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


extern bool pic_rtc_Enabled;

// Init installs the required interrupt handlers.
void pic_rtc_Init();

// SetHandler sets the interrupt handler for RTC interrupts.
//
// The handler is jumped to, so it must use IRET and save registers is uses.
void pic_rtc_SetHandler(void *handler);


#ifdef __cplusplus
}
#endif
