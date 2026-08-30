; A user-space program to test that the system calls and the ELF loading work as intended.
; Written by RPerson.

BITS 32

section .text
global _start

_start:
	; Checks wheather the kernel is comptiable
	mov eax, 0x01
	mov ebx, 0x01
	mov ecx, check_kernel_str
	mov edx, check_kernel_str_len
	int 0x84
	
	xor eax, eax
	int 0x84
	
	cmp eax, major_kernel_ver
	jne incomptiable_kernel
	cmp ebx, minor_kernel_ver
	jne incomptiable_kernel
	cmp ecx, patch_kernel_ver
	jne incomptiable_kernel
	
	mov eax, 0x01
	mov ebx, 0x01
	mov ecx, comptiable_kernel_str
	mov edx, comptiable_kernel_str_len
	int 0x84
	
	; Checks the sleep
	mov eax, 0x01
	mov ebx, 0x01
	mov ecx, sleep_str
	mov edx, sleep_str_len
	int 0x84
	
	mov eax, 0x03
	mov ebx, 5000
	int 0x84
	
	; Checks the files
	mov eax, 0x01
	mov ebx, 0x02
	mov ecx, stderr_str
	mov edx, stderr_str_len
	int 0x84
	
	mov eax, 0x01
	mov ebx, 0x01
	mov ecx, stdin_check_str
	mov edx, stdin_check_str_len
	int 0x84
	
	mov eax, 0x05
	mov ebx, 0x00
	mov ecx, stdin_buffer
	mov edx, stdin_buffer_len
	int 0x84
	
	cmp eax, 0x00
	jl stdin_error
	
	mov eax, 0x01
	mov ebx, 0x01
	mov ecx, stdin_buffer
	mov edx, stdin_buffer_printlen
	int 0x84
	
exit:
	mov ebx, 0x00
	mov eax, 0x02
	int 0x84
	jmp exit
	
incomptiable_kernel:
	mov eax, 0x01
	mov ebx, 0x01
	mov ecx, incomptiable_kernel_str
	mov edx, incomptiable_kernel_str_len
	int 0x84
	
	jmp exit
	
stdin_error:
	mov eax, 0x01
	mov ebx, 0x02
	mov ecx, stdin_error_str
	mov edx, stdin_error_str_len
	int 0x84
	
	jmp exit

section .data

major_kernel_ver equ 0x01
minor_kernel_ver equ 0x00
patch_kernel_ver equ 0x00

check_kernel_str db "Checking the NanoKernel version.", 0x0A
check_kernel_str_len equ $ - check_kernel_str

incomptiable_kernel_str db "Incomptiable NanoKernel version detected.", 0x0A
incomptiable_kernel_str_len equ $ - incomptiable_kernel_str

comptiable_kernel_str db "Detected NanoKernel version 1.0.0.", 0x0A
comptiable_kernel_str_len equ $ - comptiable_kernel_str

sleep_str db "Sleeping for 5s (5,000ms).", 0x0A
sleep_str_len equ $ - sleep_str

stderr_str db "This was printed to STDERR.", 0x0A
stderr_str_len equ $ - stderr_str

stdin_check_str db "The data read from STDIN is: "
stdin_check_str_len equ $ - stdin_check_str
stdin_buffer db 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A
stdin_buffer_len equ $ - stdin_buffer
stdin_buffer_printlen equ $ - stdin_buffer - 0x01

stdin_error_str db "Error on reading from STDIN.", 0x0A
stdin_error_str_len equ $ - stdin_error_str

section .bss
