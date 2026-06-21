; NanoOS Alpha v1.1.1 default UI's assembly code
; Written by RPserson

BITS 32

extern ui_systemcall
global __uisystemcall

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
	add esp, 20
	popa
	iretd