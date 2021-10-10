format elf64


section '.text' executable

; whatevercall void __Panic_HaltSystem();
;
; Panic_HaltSystem halts the system by an infinite loop calling the HLT instruction.
public __Panic_HaltSystem
__Panic_HaltSystem:
	hlt
	jmp __Panic_HaltSystem

