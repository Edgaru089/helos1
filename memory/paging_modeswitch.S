format elf64


section '.text' executable

; Details on Control Registers and MSRs can be found here:
; https://wiki.osdev.org/CPU_Registers_x86-64

; x64fastcall void paging_modeswitch_4LevelPaging(void* pml4, int pcid)
;
; This function assumes that the program is now in 64-bit mode (long mode).
; Paging(CR0.PG) in general and long mode (MSR EFER.LME) must already be enabled in this state.
;
; Input:   (void* rcx, int rdx)
; Clobbers: rax, flags
public paging_modeswitch_4LevelPaging
paging_modeswitch_4LevelPaging:
	; 4 Level paging: CR0.PG            (bit 31) = 1 (Protected mode enable)
	;                 CR4.PAE           (bit 05) = 1 (PAE enable)
	;                 MSR IA32_EFER.LME (bit 10) = 1 (IA32e 64-bit mode enable)
	;                 CR4.LA57          (bit 12) = 0 (4-level paging instead of 5)
	; We only need to set CR4.LA57
	; Let's also set CR4.PCIDE(bit 17)=1, enabling process-context identifiers
	mov rax, cr4
	and rax, 0xFFFFFFFFFFFFEFFF ; unset CR4.LA57
	or  rax, 0x20000    ; set CR4.PCIDE
	mov cr4, rax
	
	and rdx, 0xFFF  ; take only the 11:0 bits of the PCID
	or  rcx, rdx    ; construct the full CR3
	mov cr3, rcx    ; set CR3, invalidate all TLB cache
	ret

; x64fastcall void paging_modeswitch_4LevelPagingNX(void* pml4, int pcid)
;
; This function assumes that the program is now in 64-bit mode (long mode).
; Paging(CR0.PG) in general and long mode (MSR EFER.LME) must already be enabled in this state.
;
; This function also sets the IA32_EFER.NXE bit, enabling No-Execute feature.
;
; Input:   (void* rcx, int rdx)
; Clobbers: rax, r8, r9, flags
public paging_modeswitch_4LevelPagingNX
paging_modeswitch_4LevelPagingNX:
	; 4 Level paging: CR0.PG            (bit 31) = 1 (Protected mode enable)
	;                 CR4.PAE           (bit 05) = 1 (PAE enable)
	;                 MSR IA32_EFER.LME (bit 10) = 1 (IA32e 64-bit mode enable)
	;                 CR4.LA57          (bit 12) = 0 (4-level paging instead of 5)
	; We only need to set CR4.LA57
	; Let's also set CR4.PCIDE(bit 17)=1, enabling process-context identifiers
	mov rax, cr4
	and rax, 0xFFFFFFFFFFFFEFFF ; unset CR4.LA57
	or  rax, 0x20000    ; set CR4.PCIDE (bit 17)
	mov cr4, rax

	; save rcx and rdx, RDMSR/WRMSR uses these
	mov r8, rcx
	mov r9, rdx

	mov ecx, 0xC0000080 ; operate on the IA32_EFER MSR
	rdmsr               ; read the MSR into edx:eax
	or  eax, (1 shl 11) ; set No-Execute Enable (bit 11)
	wrmsr               ; write the MSR back

	; restore rcx and rdx
	mov rcx, r8
	mov rdx, r9
	
	and rdx, 0xFFF  ; take only the 11:0 bits of the PCID
	or  rcx, rdx    ; construct the full CR3
	mov cr3, rcx    ; set CR3, invalidate all TLB cache
	ret

; x64fastcall void paging_modeswitch_Table(void* pml, int pcid)
;
; This function simply sets CR3 and run INVLPG, flushing the TLB cache.
;
; Input:   (void* rcx, int rdx)
; Clobbers: none
public paging_modeswitch_Table
paging_modeswitch_Table:
	and rdx, 0xFFF  ; take only the 11:0 bits of the PCID
	or  rcx, rdx    ; construct the full CR3
	mov cr3, rcx    ; set CR3, invalidate all TLB cache
	ret

