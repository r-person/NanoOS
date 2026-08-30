BITS 32

section .text
global _start
extern main

_start:
	;call main
	
	mov eax, 0x01
	mov ebx, 0x01
	mov ecx, hello_str
	mov edx, hello_len
	int 0x84
	
	cmp eax, hello_len
	jne exit_err
	mov ebx, 0x00
	mov eax, 0x02
	int 0x84

exit_err:
	mov ebx, -0x01
	mov eax, 0x02
	int 0x84
	jmp _start

section .data

hello_str db "Hello world!", 0x0A
hello_len equ $ - hello_str

section .bss
