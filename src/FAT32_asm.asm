; The NanoOS's FAT32 default loader's assembly code
; Written by RPerson

BITS 32

extern fat32_systemcall
global __filesystemsystemcall

section .data
section .text

__filesystemsystemcall:
	pusha
	push esp
	push edx
	push ecx
	push ebx
	push eax
	call fat32_systemcall
	add esp, 20
	popa
	iretd
