BITS 32

section .text
global _start
extern main

_start:
	call main
	
	mov ebx, eax
	mov eax, 0x02
	int 0x84
	jmp _start

section .data

section .bss
