format elf64

public interrupt_ReloadSegments
public interrupt_LoadGDT
public interrupt_LoadIDT


section '.text' executable

; sysvx64call void interrupt_LoadGDT(void* gdtr)
;
; Input:    (void* rdi)
; Clobbers: none
interrupt_LoadGDT:
	lgdt [rdi]
	ret

; sysvx64call void interrupt_ReloadSegments()
;
; Clobbers: rax
interrupt_ReloadSegments:
	mov eax, 0x10  ; my data segment
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	;jmp  0x08:.flush
	; as in https://forum.osdev.org/viewtopic.php?f=1&t=30739
	; farjump does not work in long mode, you need to do a far return:
	pop  rax
	push qword 0x08  ; my code segment
	push rax
	retfq

; sysvx64call void interrupt_LoadIDT(void* idtr)
;
; Input:    (void* rdi)
; Clobbers: none
interrupt_LoadIDT:
	lidt [rdi]
	ret

