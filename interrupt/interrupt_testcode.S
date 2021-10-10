format elf64

extrn io_WriteConsoleASCII


section '.rodata'
interrupt_string:
	db "Interrupt Testcode", 0x0A, 0x00


section '.data' writable
align 4
idt:
	rb 50*16

idtr:
	dw (50*16)-1
	dq idt


section '.text' executable
int_handler:
	push rax
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11
	sub  rsp, 32
	mov  rcx, interrupt_string
	call io_WriteConsoleASCII
	add  rsp, 32
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdx
	pop rcx
	pop rax
	iretq


public interrupt_Testcode
interrupt_Testcode:
	lidt [idtr]
	mov rax, int_handler
	mov [idt+49*16], ax
	mov word [idt+49*16+2], 0x08
	mov word [idt+49*16+4], 0x8e00
	shr rax, 16
	mov [idt+49*16+6], ax
	shr rax, 16
	mov [idt+49*16+8], rax

	int 49
	ret
