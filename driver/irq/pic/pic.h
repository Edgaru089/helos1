#pragma once

#include "../../../main.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


// Default location for PIC INT/IRQ remap
#define IRQ_PIC_INT_OFFSET_MASTER 0x20 // INT20h...INT27h
#define IRQ_PIC_INT_OFFSET_SLAVE  0x28 // INT28h...INT2fh

// Init PIC, mask all irqs
void irq_pic_Init();

// remap Master PIC to INToff_master...+7, Slave to INToff_slave...+7
void irq_pic_InitRemap(int offset_master, int offset_slave);

// Mask(disable) / Unmask(enable) certain IRQ line
void irq_pic_Mask(uint8_t irq_line, bool masked);

// Disable legacy PIC (masks all IRQ lines)
SYSV_ABI void irq_pic_Disable(); // defined in assembly

// Get the combined 16 bits of Interrupt-Request-Register and In-Service-Register
uint16_t irq_pic_GetIRR();
uint16_t irq_pic_GetISR();


// void(uintptr_t) for IRQ handlers, no need to call out8(OCW2, 0x6*)
typedef SYSV_ABI void (*irq_pic_IRQHandlerType)(uintptr_t);

// defined in pic_init.c
extern void     *irq_pic_IRQHandler[16];
extern uintptr_t irq_pic_IRQHandler_Data[16]; // written into RDI on handler (first argument)
extern bool      irq_pic_Enabled;

// If IRQHandlerRaw[irq] is not NULL, the function is jumped to (not called).
//
// So the handler should use IRET instead of RET, and save all the registers it uses.
extern void *irq_pic_IRQHandlerRaw[16];


#ifdef __cplusplus
}
#endif
