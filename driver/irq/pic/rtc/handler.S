format elf64

public __pic_rtc_IRQ8
public __pic_rtc_IRQHandler;


section '.bss' writable
__pic_rtc_IRQHandler:
	rq 1

section '.text' executable
__pic_rtc_IRQ8:
	push rax

	; read the register C byte or the interrupt will block
	mov al, 0x0c
	out 0x70, al
	in  al, 0x71   

	; tell if the irq handler is NULL
	mov  rax, [__pic_rtc_IRQHandler]
	test rax, rax
	pop  rax
	jz  .end
	jmp qword [__pic_rtc_IRQHandler]
.end:
	iret


