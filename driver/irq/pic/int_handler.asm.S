format elf64

include 'pic_constants.incS'

extrn irq_pic_IntHandler
extrn irq_pic_IRQHandlerRaw

public irq_pic_IntHandler20h
public irq_pic_IntHandler21h
public irq_pic_IntHandler22h
public irq_pic_IntHandler23h
public irq_pic_IntHandler24h
public irq_pic_IntHandler25h
public irq_pic_IntHandler26h
public irq_pic_IntHandler27h
public irq_pic_IntHandler28h
public irq_pic_IntHandler29h
public irq_pic_IntHandler2ah
public irq_pic_IntHandler2bh
public irq_pic_IntHandler2ch
public irq_pic_IntHandler2dh
public irq_pic_IntHandler2eh
public irq_pic_IntHandler2fh


section '.text' executable

macro inth op1 {
	push rax

	mov  rax, [irq_pic_IRQHandlerRaw+op1*8]
	test rax, rax
	jz .noraw

	; has raw handler
	mov al, PIC_CMD_EOI
	if op1 >= 8
		out PIC2_COMMAND, al
	end if
	out PIC1_COMMAND, al

	pop rax
	jmp qword [irq_pic_IRQHandlerRaw+op1*8]

.noraw:
	push rdi
	mov rdi, op1
	push rsi
	push rdx
	push rcx
	push r8
	push r9
	push r10
	push r11
	call irq_pic_IntHandler
	pop r11
	pop r10
	pop r9
	pop r8
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	pop rax
	iretq
}

irq_pic_IntHandler20h:
	inth 0
irq_pic_IntHandler21h:
	inth 1
irq_pic_IntHandler22h:
	inth 2
irq_pic_IntHandler23h:
	inth 3
irq_pic_IntHandler24h:
	inth 4
irq_pic_IntHandler25h:
	inth 5
irq_pic_IntHandler26h:
	inth 6
irq_pic_IntHandler27h:
	inth 7
irq_pic_IntHandler28h:
	inth 8
irq_pic_IntHandler29h:
	inth 9
irq_pic_IntHandler2ah:
	inth 10
irq_pic_IntHandler2bh:
	inth 11
irq_pic_IntHandler2ch:
	inth 12
irq_pic_IntHandler2dh:
	inth 13
irq_pic_IntHandler2eh:
	inth 14
irq_pic_IntHandler2fh:
	inth 15

