format elf64


section '.text' executable

; x64fastcall void* memset(void* dest, int data, size_t count)
;
; Input:   (void* rcx, int rdx, size_t r8)
; Output:   void* rax
; Clobbers: r10, flags
public memset
memset:
	mov rax, rcx
	lea r10, [rcx+r8]

memset_loop:
	mov byte[rcx], dl
	inc rcx
	cmp rcx, r10
	jne memset_loop

	ret

; x64fastcall void* memmove(void* dest, const void* src, size_t count)
;
; Input:   (void* rcx, void* rdx, size_t r8)
; Output:   void* rax
; Clobbers: r9, r10, r11, flags
public memmove
memmove:
	mov rax, rcx

	lea r10, [rcx+r8]  ; past-the-end for *dest
	lea r11, [rdx+r8]  ; past-the-end for *src

	cmp rdx, rcx
	je  memmove_end    ; return if move buffers are the same
	jl  memmove_back   ; *src < *dest: overlaps, copy backward

memmove_front:         ; *src > *dest: overlaps, copy forward
	mov r9b, byte[rdx]
	mov byte[rcx], r9b
	inc rcx
	inc rdx
	cmp rcx, r10
	jne memmove_front

	jmp memmove_end

memmove_back:
	dec r10
	dec r11
	mov r9b, byte[r11]
	mov byte[r10], r9b
	cmp rcx, r10
	jne memmove_back

memmove_end:
	ret

