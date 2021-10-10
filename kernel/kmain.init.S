format elf64

extrn kMain

public kMain_StackPosition
public kMain_Init


section '.bss' writable
kMain_StackPosition:
	rq 1


section '.text' executable

; sysvx64call void kMain_Init()
kMain_Init:
	mov  rsp, [kMain_StackPosition]
	call kMain

.hlt:
	hlt
	jmp .hlt


