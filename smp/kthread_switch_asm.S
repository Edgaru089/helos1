format elf64

extrn  __smp_Switch
extrn  __smp_PauseTicker
extrn  __smp_Now
public __smp_IntSwitch_LastState
public __smp_IntSwitch
public __smp_Switch_Idle
public smp_thread_Yield


section '.bss' writable

; typedef struct {
;     uint64_t rax, rbx, rcx, rdx;
;     uint64_t rsi, rdi, rbp, rsp;
;     uint64_t r8, r9, r10, r11;
;     uint64_t r12, r13, r14, r15;
; 
;     uint64_t rflags;
;     uint64_t rip;
;     uint16_t ss, cs; // Stack and Code Segments
; } PACKED smp_thread_State;
__smp_IntSwitch_LastState:
	rq 18
	rw 2


section '.text' executable

; sysvx64abi smp_thread_Yield()
;
; Sets __smp_PauseTicker.
; Pushes SS, RSP, RFLAGS, CS, RIP into the stack as Qwords,
; like what the CPU does on execption,
; and jumps to __smp_IntSwitch.
smp_thread_Yield:
	mov byte [__smp_PauseTicker], 1
	int 0x28
	ret


; interrupt_handler smp_IntSwitch()
;
; Called from a timer interrupt.
; Saves the current processor (general-purpose) state and invoke the task switcher.
__smp_IntSwitch:
	cli

	mov [__smp_IntSwitch_LastState],    rax

	; if (!__smp_PauseTicker) {
	;     __smp_Now++;
	;     if (__smp_Now % 8)
	;         return 0;
	; } else
	;     __smp_PauseTicker = false;
	mov  al, [__smp_PauseTicker]
	test al, al
	jnz  .noticker
	
	mov  rax, [__smp_Now]
	inc  rax
	mov  [__smp_Now], rax
	test rax, 0x3
	jz   .realcall
	mov  rax, [__smp_IntSwitch_LastState]
	iret

.noticker:
	mov byte [__smp_PauseTicker], 0

.realcall:
	mov [__smp_IntSwitch_LastState+8],  rbx
	mov [__smp_IntSwitch_LastState+16], rcx
	mov [__smp_IntSwitch_LastState+24], rdx
	mov [__smp_IntSwitch_LastState+32], rsi

	; now that we have 5 free registers, pop the iret flags for later
	pop rax ; rip
	pop rbx ; cs
	pop rcx ; rflags
	pop rdx ; rsp
	pop rsi ; ss

	mov [__smp_IntSwitch_LastState+40], rdi
	mov [__smp_IntSwitch_LastState+48], rbp
	mov [__smp_IntSwitch_LastState+56], rdx ; rsp
	mov [__smp_IntSwitch_LastState+64], r8
	mov [__smp_IntSwitch_LastState+72], r9
	mov [__smp_IntSwitch_LastState+80], r10
	mov [__smp_IntSwitch_LastState+88], r11
	mov [__smp_IntSwitch_LastState+96], r12
	mov [__smp_IntSwitch_LastState+104], r13
	mov [__smp_IntSwitch_LastState+112], r14
	mov [__smp_IntSwitch_LastState+120], r15

	mov [__smp_IntSwitch_LastState+128], rcx ; rflags
	mov [__smp_IntSwitch_LastState+136], rax ; rip
	mov [__smp_IntSwitch_LastState+144], si  ; ss
	mov [__smp_IntSwitch_LastState+146], bx  ; cs

	call __smp_Switch
	;test rax, rax
	;jnz  .realswitch  ; nonzero return value means really a context switch

.realswitch:
	mov rdi, [__smp_IntSwitch_LastState+40]
	mov rbp, [__smp_IntSwitch_LastState+48]
	mov rdx, [__smp_IntSwitch_LastState+56] ; rsp
	mov r8,  [__smp_IntSwitch_LastState+64]
	mov r9,  [__smp_IntSwitch_LastState+72]
	mov r10, [__smp_IntSwitch_LastState+80]
	mov r11, [__smp_IntSwitch_LastState+88]
	mov r12, [__smp_IntSwitch_LastState+96]
	mov r13, [__smp_IntSwitch_LastState+104]
	mov r14, [__smp_IntSwitch_LastState+112]
	mov r15, [__smp_IntSwitch_LastState+120]

	mov rcx, [__smp_IntSwitch_LastState+128] ; rflags
	mov rax, [__smp_IntSwitch_LastState+136] ; rip
	mov si,  [__smp_IntSwitch_LastState+144] ; ss
	mov bx,  [__smp_IntSwitch_LastState+146] ; cs

	push rsi ; ss
	push rdx ; rsp
	push rcx ; rflags
	push rbx ; cs
	push rax ; rip

	mov rax, [__smp_IntSwitch_LastState]
	mov rbx, [__smp_IntSwitch_LastState+8]
	mov rcx, [__smp_IntSwitch_LastState+16]
	mov rdx, [__smp_IntSwitch_LastState+24]
	mov rsi, [__smp_IntSwitch_LastState+32]

	iret


__smp_Switch_Idle:
	hlt
	jmp __smp_Switch_Idle
	jmp __smp_Switch_Idle
	ret

