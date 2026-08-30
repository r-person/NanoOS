; NanoKernel's Assembly code
; Written by RPerson

BITS 32

NUMBER_OF_PAGE_TABLES equ 0x04

section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00000003
    dd -(0x1BADB002 + 0x00000003)

; the code to enable paging
section .boot.text
global start
global page_directory
global kernel_page_tables
extern set_paging
extern build_memory_map

prepaging_panic:
	cli
	hlt
	jmp prepaging_panic

start:
	mov esp, prepaging_stack_top
	cmp eax, 0x2BADB002
	jne prepaging_panic
	
	push ebx
	call build_memory_map
	
	push kernel_page_tables
	push page_directory
	call set_paging

enable_paging:
	mov eax, page_directory
	mov cr3, eax

	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	
	jmp __start

section .boot.bss
align 16
prepaging_stack_bottom:
	resb 0x800
prepaging_stack_top:

align 0x1000
page_directory:
	resb 0x1000
align 0x1000
kernel_page_tables:
	resb 0x1000 * NUMBER_OF_PAGE_TABLES ; Page table size * number of tables

global __start
global gdt_flush
global enable_paging
global halt
global stack_bottom
global stack_top
global isr8
global isr32
global __systemcall
global __intersystemcall
global __linux_syscall
global thread_jmp
global __hlt
extern kernel_main
extern updated_page_directory
extern isr_handler
extern systemcall
extern intersystemcall
extern linux_systemcall
extern irq0_handler

section .text
__start:
	mov esp, stack_top
	call kernel_main

__hlt:
	sti
	hlt
	jmp __hlt

gdt_flush:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp dword 0x08:flush2

flush2:
    ret

panic:
	cli
	hlt
	jmp panic

thread_jmp:
	push dword 0x23			; SS
	push dword 0x003FFFFF	; ESP
	
	pushf
	pop eax
	xor eax, 0000001000000000b
	push eax				; EFLAGS
	
	push dword 0x1B			; CS
	push dword ebx			; EIP
	
	mov ax, 0x23
	mov gs, ax
	mov fs, ax
	mov es, ax
	mov ds, ax
	
	iret

%macro ISR_HANDLER 1

global isr%1

isr%1:
    cli
	
	push ds
	push es
	push fs
	push gs
	push eax
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	pop eax
	
    pushad
	
	push dword [esp + 0x34]
	push dword 0x00
    push dword %1
    call isr_handler
    add esp, 0x0C
	
    popad
	pop gs
	pop fs
	pop es
	pop ds
    iretd

%endmacro

%macro ERR_CODE_ISR_HANDLER 1

global isr%1

isr%1:
    cli
	
	push ds
	push es
	push fs
	push gs
	
	push eax
	
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
	pop eax
    pushad
	
	push dword [esp + 0x34]
	push dword [esp + 0x34]
    push dword %1
    call isr_handler
    add esp, 0x0C
	
    popad
	pop gs
	pop fs
	pop es
	pop ds
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
	mov ax, 0x10
	mov ds, ax
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

isr32:
	cli
	pushad
	push gs
	push fs
	push es
	push ds
	pushf
	call irq0_handler
	popf
	pop ds
	pop es
	pop fs
	pop gs
	popad
	iret

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

__linux_syscall:
	pushad
	push esp
	push edx
	push ecx
	push ebx
	push eax
	call linux_systemcall
	add esp, 20
	popad
	iretd

__systemcall:
	pushad
	push esp
	push edx
	push ecx
	push ebx
	push eax
	call systemcall
	add esp, 20
	popad
	iretd

__intersystemcall:
	pushad
	push esp
	push edx
	push ecx
	push ebx
	push eax
	call intersystemcall
	add esp, 20
	popad
	iretd

section .data

section .bss
align 0x10
stack_bottom:
    resb 0x4000
stack_top:
triple_fault_idt_ptr:
    dw 0x00
    dd 0x00

section .data