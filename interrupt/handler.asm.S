format elf64

; sysvx64call void interrupt_Handler(a, b, c, d, e, f)
extrn interrupt_Handler
; sysvx64call void interrupt_Handler128(a, b, c, d, e, f)
; Input: rax(syscall opcode)
extrn interrupt_Handler128

extrn io_WriteConsoleASCII

public interrupt_Int0
public interrupt_Int1
public interrupt_Int2
public interrupt_Int3
public interrupt_Int4
public interrupt_Int5
public interrupt_Int6
public interrupt_Int7
public interrupt_Int8
public interrupt_Int9
public interrupt_Int10
public interrupt_Int11
public interrupt_Int12
public interrupt_Int13
public interrupt_Int14
public interrupt_Int15
public interrupt_Int16
public interrupt_Int17
public interrupt_Int18
public interrupt_Int19
public interrupt_Int20
public interrupt_Int21
public interrupt_Int22
public interrupt_Int23
public interrupt_Int24
public interrupt_Int25
public interrupt_Int26
public interrupt_Int27
public interrupt_Int28
public interrupt_Int29
public interrupt_Int30
public interrupt_Int31

public interrupt_Int128


section '.text' executable


macro inth op1 {
	push rdi
	mov rdi, op1
	push rsi
	push rdx
	mov rdx, [rsp+24]
	push rcx
	push r8
	push r9
	push r10
	push r11
	push rax
	call interrupt_Handler
	pop rax
	pop r11
	pop r10
	pop r9
	pop r8
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	iretq
}

macro inth_err op1 {
	push rsi
	push rdi
	mov rdi, op1
	mov esi, [rsp+16]
	push rdx
	mov rdx, [rsp+32]
	push rcx
	push r8
	push r9
	push r10
	push r11
	push rax
	call interrupt_Handler
	pop rax
	pop r11
	pop r10
	pop r9
	pop r8
	pop rcx
	pop rdx
	pop rdi
	pop rsi
	add rsp, 8 ; pop the error code
	iretq
}

interrupt_Int0: ; does not return
	inth 0
interrupt_Int1:
	inth 1
interrupt_Int2:
	inth 2
interrupt_Int3:
	inth 3
interrupt_Int4:
	inth 4
interrupt_Int5:
	inth 5
interrupt_Int6:
	inth 6
interrupt_Int7:
	inth 7
interrupt_Int8:
	inth 8
interrupt_Int9:
	inth 9
interrupt_Int10:
	inth_err 10
interrupt_Int11:
	inth_err 11
interrupt_Int12:
	inth_err 12
interrupt_Int13:
	inth_err 13
interrupt_Int14:
	inth_err 14
interrupt_Int15:
	inth 15
interrupt_Int16:
	inth 16
interrupt_Int17:
	inth_err 17
interrupt_Int18:
	inth 18
interrupt_Int19:
	inth 19
interrupt_Int20:
	inth 20
interrupt_Int21:
	inth_err 21
interrupt_Int22:
	inth 22
interrupt_Int23:
	inth 23
interrupt_Int24:
	inth 24
interrupt_Int25:
	inth 25
interrupt_Int26:
	inth 26
interrupt_Int27:
	inth 27
interrupt_Int28:
	inth 28
interrupt_Int29:
	inth 29
interrupt_Int30:
	inth 30
interrupt_Int31:
	inth 31

interrupt_Int128:
	;sub  rsp, 32
	;mov  rcx, interrupt_string
	;call io_WriteConsoleASCII
	;add  rsp, 32
	;iretq

	; no need to save the registers
	;push rax
	;push rdi
	;push rsi
	;push rdx
	;push rcx
	;push r8
	;push r9
	;push r10
	;push r11
	call interrupt_Handler128
	;pop r11
	;pop r10
	;pop r9
	;pop r8
	;pop rcx
	;pop rdx
	;pop rsi
	;pop rdi
	;pop rax
	iretq
