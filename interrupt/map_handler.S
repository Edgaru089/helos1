format elf64

extrn io_WriteConsoleASCII

public interrupt_MapHandler


section '.text' executable

; sysvx64call void interrupt_MapHandler(uint64_t handler, int interrupt)
;
; Input:   (uint64_t rdi, int rsi)
; Clobbers: rax, flags
interrupt_MapHandler:
	mov rax, 0xFFFFFFFEC0000000  ; KERNEL_IDT_MAPPING
	shl rsi, 4                   ; rsi *= 16
	add rsi, rax                 ; rsi += KERNEL_IDT_MAPPING
	mov [rsi], di
	mov word [rsi+2], 0x08       ; GDT_EXEC_SELECTOR (index=1)
	mov word [rsi+4], 0x8e00
	shr rdi, 16
	mov [rsi+6], di
	shr rdi, 16
	mov [rsi+8], rdi
	ret
