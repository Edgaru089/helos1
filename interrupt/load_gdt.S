format elf64

public interrupt_ReloadSegments
public interrupt_LoadGDT
public interrupt_LoadIDT


section '.text' executable

; sysvx64call void interrupt_ReloadSegments()
;
; Clobbers: rax
interrupt_ReloadSegments:
	mov ax, 0x08  ; my data segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	;jmp  0x10:.flush
	; as in https://forum.osdev.org/viewtopic.php?f=1&t=30739
	; farjump does not work in long mode, you need to do a far return:
	pop  rax
	push qword 0x10  ; my code segment
	push rax
	retfq

; sysvx64call void interrupt_LoadGDT(uint16_t length_sub1, void* base_ptr)
;
; Input:  (uint16_t di, void* rsi)
; Clobbers: rax, flags
interrupt_LoadGDT:
	mov  rax, rsp  ; save old RSP
	sub  rsp, 10
	and  rsp, 0xfffffffffffffffc  ; align RSP to 4-byte
	mov  [rsp], di
	mov  [rsp+2], rsi
	lgdt [rsp]
	mov  rsp, rax  ; restore old RSP
	ret

; sysvx64call void interrupt_LoadIDT(uint16_t length_sub1, void* base_ptr)
;
; Input:  (uint16_t di, void* rsi)
; Clobbers: rax, flags
interrupt_LoadIDT:
	mov  rax, rsp  ; save old RSP
	sub  rsp, 10
	and  rsp, 0xfffffffffffffffc  ; align RSP to 4-byte
	mov  [rsp], di
	mov  [rsp+2], rsi
	lidt [rsp]
	mov  rsp, rax  ; restore old RSP
	ret
