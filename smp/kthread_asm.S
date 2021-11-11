format elf64

extrn  __smp_thread_Cleanup
public __smp_thread_EntryPoint


; sysvx64abi void __smp_thread_EntryPoint(uintptr_t a,b,c,d,e,f)
;
; Calls thread entry point in RAX, and, after it returns,
; removes the current thread from the structure.
section '.text' executable
__smp_thread_EntryPoint:
	call rax

	; Thread returned; delete the current thread
	; The current thread: In __smp_Threads, not in __smp_ThreadsWaiting

	cli  ; Disable interrupt on thread structure operations
	jmp __smp_thread_Cleanup  ; This will not return
