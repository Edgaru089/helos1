format elf64

public asm_Syscall as 'Syscall'   ; syscall is a reserved token

section ".text" executable

; sysvx64call int Syscall(int syscall_id, int a,b,c,d,e,f)
asm_Syscall:
	mov rax, rdi
	mov rdi, rsi
	mov rsi, rdx
	mov rdx, rcx
	mov rcx, r8
	mov r8,  r9
	mov r9,  [rsp+8]
	int 0x80
	ret

