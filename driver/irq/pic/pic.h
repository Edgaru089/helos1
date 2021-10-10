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


// void() for IRQ handlers, no need to call out8(OCW2, 0x6*)
typedef SYSV_ABI void (*irq_pic_IRQHandlerType)();

// defined in pic_init.c
extern irq_pic_IRQHandlerType irq_pic_IRQHandler[16];
extern bool                   irq_pic_Enabled;


#ifdef __cplusplus
}
#endif
