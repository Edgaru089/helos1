#pragma once

#include "../../../main.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


static inline void outb(uint16_t port, uint8_t val) {
	asm volatile("outb %0, %1" ::"a"(val), "Nd"(port));
	/*There's an outb %al, $imm8 encoding, for compile-time constant port numbers that fit in 8b. (N constraint).
      Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
      The outb %al, %dx encoding is the only option for all other cases.
      %1 expands to %dx because port is a uint16_t. %w1 could be used if we had the port number a wider C type*/
}

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile("inb %1, %0"
				 : "=a"(ret)
				 : "Nd"(port));
	return ret;
}

// Wait a very small amount of time (1 to 4 microseconds, generally).
static inline void io_wait(void) {
	outb(0x80, 0);
}

static inline void outb_wait(uint16_t port, uint8_t val) {
	asm volatile("outb %0, %1" ::"a"(val), "Nd"(port));
	/*There's an outb %al, $imm8 encoding, for compile-time constant port numbers that fit in 8b. (N constraint).
      Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
      The outb %al, %dx encoding is the only option for all other cases.
      %1 expands to %dx because port is a uint16_t. %w1 could be used if we had the port number a wider C type*/
	asm volatile("outb %al, $0x80");
}


#define PIC1         0x20 // IO base address for master PIC
#define PIC2         0xA0 // IO base address for slave PIC
#define PIC1_COMMAND 0x20 // Port for Command for master PIC
#define PIC1_DATA    0x21 // Port for Data    for master PIC
#define PIC2_COMMAND 0xA0 // Port for Command for slave PIC
#define PIC2_DATA    0xA1 // Port for Data    for slave PIC

#define PIC_CMD_EOI 0x20 // End-Of-Interrupt command

#define ICW1_ICW4      0x01 // ICW4 (not) needed
#define ICW1_SINGLE    0x02 // Single (cascade) mode
#define ICW1_INTERVAL4 0x04 // Call address interval 4 (8)
#define ICW1_LEVEL     0x08 // Level triggered (edge) mode
#define ICW1_INIT      0x10 // Initialization - required!

#define ICW4_8086       0x01 // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO       0x02 // Auto (normal) EOI
#define ICW4_BUF_SLAVE  0x08 // Buffered mode/slave
#define ICW4_BUF_MASTER 0x0C // Buffered mode/master
#define ICW4_SFNM       0x10 // Special fully nested (not)


SYSV_ABI void irq_pic_IntHandler(int irq);

SYSV_ABI void irq_pic_IntHandler20h();
SYSV_ABI void irq_pic_IntHandler21h();
SYSV_ABI void irq_pic_IntHandler22h();
SYSV_ABI void irq_pic_IntHandler23h();
SYSV_ABI void irq_pic_IntHandler24h();
SYSV_ABI void irq_pic_IntHandler25h();
SYSV_ABI void irq_pic_IntHandler26h();
SYSV_ABI void irq_pic_IntHandler27h();
SYSV_ABI void irq_pic_IntHandler28h();
SYSV_ABI void irq_pic_IntHandler29h();
SYSV_ABI void irq_pic_IntHandler2ah();
SYSV_ABI void irq_pic_IntHandler2bh();
SYSV_ABI void irq_pic_IntHandler2ch();
SYSV_ABI void irq_pic_IntHandler2dh();
SYSV_ABI void irq_pic_IntHandler2eh();
SYSV_ABI void irq_pic_IntHandler2fh();


#ifdef __cplusplus
}
#endif
