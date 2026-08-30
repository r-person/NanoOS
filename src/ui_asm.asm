; NanoOS default UI's assembly code
; Written by RPserson

BITS 32

extern ui_systemcall
extern ui_start
extern ui_main
global __uisystemcall
global _start

section .data
section .text

__uisystemcall:
	pusha
	push esp
	push edx
	push ecx
	push ebx
	push eax
	call ui_systemcall
	add esp, 0x14
	popa
	iretd

_start:
	mov eax, 0x02
	xor ebx, ebx
	mov bl, 0x82
	mov ecx, __uisystemcall
	call ui_start

mainloop:
	mov eax, 0x03
	mov ebx, 0x01
	int 0x84
	call ui_main
	jmp mainloop
