; NanoKernel v0.1.0's Assembly code
; Written by RPerson

BITS 32

global _start
global gdt_flush
global enable_paging
extern page_directory
extern kernel_main

section .multiboot
align 4

MBALIGN  equ 1<<0
MEMINFO  equ 1<<1
FLAGS    equ MBALIGN | MEMINFO
MAGIC    equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

dd MAGIC
dd FLAGS
dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .text

_start:
    cli
	mov esp, stack_top

    call kernel_main

.hang:
    hlt
    jmp .hang

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

enable_paging:
	mov eax, page_directory
	mov cr3, eax
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	ret
