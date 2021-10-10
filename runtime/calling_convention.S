format elf64

section '.text' executable

; x64sysvcall int sysv_x64fastcall(void* addr, int numArgs, long args1, args2, args3, args4)
;
; Calls Microsoft x64 ABI functions under System V AMD64 ABI.
;
; This function can handle ONLY up to FOUR arguments.
; numArgs is in fact unused.
;
; Input:   (void* rdi, int rsi, long rdx, rcx, r8, r9)
; Output:   int rax
; Clobbers: flags
public sysv_x64fastcall
sysv_x64fastcall:
	sub rsp, 4*8  ; reserve the 4*8 bytes of shadow space

	; So, before we start, let's make a chart!
	;
	;     | SysVx64       | MSx64         |
	; ----+---------------+---------------+
	; RAX | Return Value  | Return Value  |
	; RBX | Callee Saved  | Callee Saved  |
	; RCX | Argument 4    | Argument 1    |
	; RDX | Argument 3    | Argument 2    |
	; RSI | Argument 2    | Callee Saved  |
	; RDI | Argument 1    | Callee Saved  |
	; RBP | Callee Saved  | Callee Saved  |
	; RSP | Stack Pointer | Stack Pointer |
	; R8  | Argument 5    | Argument 3    |
	; R9  | Argument 6    | Argument 4    |
	; R10 | Caller Saved  | Caller Saved  |
	; R11 | Caller Saved  | Caller Saved  |
	; R12 | Callee Saved  | Callee Saved  |
	; R13 | Callee Saved  | Callee Saved  |
	; R14 | Callee Saved  | Callee Saved  |
	; R15 | Callee Saved  | Callee Saved  |
	;
	; To sum up, all we need to do is:
	;   - RCX = RDX; RDX = RCX; and that's it!
	; Other stuff correspond quite well actually.

	mov rax, rcx
	mov rcx, rdx
	mov rdx, rax

	call rdi

	add rsp, 4*8  ; pop the shadow space
	ret


