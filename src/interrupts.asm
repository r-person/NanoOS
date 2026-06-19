; NanoKernel v0.1.0 interrupts entery
; Written by RPerson

BITS 32

global isr8
global __systemcall
global __intersystemcall
extern isr_handler
extern systemcall
extern intersystemcall

section .data
triple_fault_idt_ptr:
    dw 0
    dd 0

section .text

%macro ISR_HANDLER 1

global isr%1

isr%1:
    cli
    pusha

    push dword %1
    call isr_handler
    add esp, 4

    popa
    iretd

%endmacro

%macro ERR_CODE_ISR_HANDLER 1

global isr%1

isr%1:
    cli
    pusha

    push dword %1
    call isr_handler
    add esp, 4

    popa
    iretd

%endmacro

ISR_HANDLER 0
ISR_HANDLER 1
ISR_HANDLER 2
ISR_HANDLER 3
ISR_HANDLER 4
ISR_HANDLER 5
ISR_HANDLER 6
ISR_HANDLER 7

isr8:
	; Resets using a triple fault
    cli
	lidt [triple_fault_idt_ptr]
	int 0x08
	hlt

ISR_HANDLER 9
ERR_CODE_ISR_HANDLER 10
ERR_CODE_ISR_HANDLER 11
ERR_CODE_ISR_HANDLER 12
ERR_CODE_ISR_HANDLER 13
ERR_CODE_ISR_HANDLER 14
ISR_HANDLER 15
ISR_HANDLER 16
ISR_HANDLER 17
ISR_HANDLER 18
ISR_HANDLER 19
ISR_HANDLER 20
ISR_HANDLER 21
ISR_HANDLER 22
ISR_HANDLER 23
ISR_HANDLER 24
ISR_HANDLER 25
ISR_HANDLER 26
ISR_HANDLER 27
ISR_HANDLER 28
ISR_HANDLER 29
ISR_HANDLER 30
ISR_HANDLER 31
ISR_HANDLER 32
ISR_HANDLER 33
ISR_HANDLER 34
ISR_HANDLER 35
ISR_HANDLER 36
ISR_HANDLER 37
ISR_HANDLER 38
ISR_HANDLER 39
ISR_HANDLER 40
ISR_HANDLER 41
ISR_HANDLER 42
ISR_HANDLER 43
ISR_HANDLER 44
ISR_HANDLER 45
ISR_HANDLER 46
ISR_HANDLER 47
ISR_HANDLER 48

__systemcall:
	pusha
	push esp
	push edx
	push ecx
	push ebx
	push eax
	call systemcall
	add esp, 20
	popa
	iretd

__intersystemcall:
	pusha
	push esp
	push edx
	push ecx
	push ebx
	push eax
	call intersystemcall
	add esp, 20
	popa
	iretd