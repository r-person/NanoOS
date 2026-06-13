# NanoOS

NanoOS is a 32-bits x86 OS built with the goal of having a small core and the possibility of expanding it in mind. 

# How to run it?

Use make for it. You can compile it yourself or use the prebuilt ISO. You need the next tools to compile it:

- Clang

- LLD

- NASM

- GNU GRUB

- xorisso

And to run it you need:

- Any kind of a VM. This project is built assuming QEMU, but the ISO can work on other VMs. 

To compile it use:

```bash
make clear
make
```

And to run use: 

```bash
make run
```

# Supported features

Correctly (as of Alpha v1.0.0) the OS supports the next features:

- Booting using GRUB into Protected Mode.

- A basic text-mode shell with the commands 'version', 'reboot' and 'echo'.

- Informing the user about CPU exception (except double faults) and halting without crashing. In case of a double fault the kernel resets the computer. In case of IRQ1 the kernel gets the keyboard input. In case of IRQ0 nothing meaningful happens, and in case of IRQ2 - IRQ15 nothing happens. The OS is also capable of handling syscalls (int 0x80), internal syscalls (int 0x81) and UI syscalls (int 0x82). 

# FAQ

## How to install apps on it?

NanoOS Alpha v1.0.0 doesn't support running apps. 

## Do you plan to add support for apps?

Yes, app support is planned in future versions. 

## Can I run it on real hardware?

The OS was tested only on QEMU, but it doesn't mean it can't run on real hardware. Just make sure that the CPU is x86 and supports 32-bits. 

## How to mod it?

To mod it you can simply edit the source code and then compile as explained before. If you want to replace the UI you need to take in mind that __uisystemcall is the entery of int 0x82 and you need to handle all the UI system calls.

# Architecture overview

The OS is built for 32-bits x86. It boots using GRUB into a simple shell. Correctly the OS does NOT support any filesystems or apps, even though support for these is planned. 

# Syscalls

System get up to 3 arguments in the registers and return up to 3 output values in the registers. If there are less than 3 output values the unused registers stay unchanged. 

## Int 0x80

Int 0x80 is the kernel syscalls. This will be the only visible system calls interrupt visible to apps when they will be added.

| Syscall number (EAX) |             goal             |       arg 1 (EBX)       | arg 2 (ECX) | arg 3 (EDX) |    output 1 (EAX)    |    output 2 (EBX)    |    output 3 (ECX)    |
|----------------------|------------------------------|-------------------------|-------------|-------------|----------------------|-----------------------|----------------------|
|         0x00         | Get the NanoKernel version   |                         |             |             | First version number | Second version number | Third version number |
|         0x01         | Prints a line to the screen  | a pointer to the string |             |             |                      |                       |                      |
|                      |                              |                         |             |             |                      |                       |                      |

## Int 0x81

Int 0x81 is the internal kernel syscalls.

| Syscall number (EAX) |      goal      | arg 1 (EBX) | arg 2 (ECX) | arg 3 (EDX) | output 1 (EAX) | output 2 (EBX) | output 3 (ECX) |
|----------------------|----------------|-------------|-------------|-------------|----------------|----------------|----------------|
|         0x00         | Reboot request |             |             |             |                |                |                |

## Int 0x82

Int 0x82 is the internal UI syscalls.

| Syscall number (EAX) |               goal               |      arg 1 (EBX)      | arg 2 (ECX) | arg 3 (EDX) | output 1 (EAX) | output 2 (EBX) | output 3 (ECX) |
|----------------------|----------------------------------|-----------------------|-------------|-------------|----------------|----------------|----------------|
|         0x00         | Setting up the UI                |                       |             |             |                |                |                |
|         0x01         | Frame (called every system tick) |                       |             |             |                |                |                |
|         0x02         | Getting an input character       | The char              |             |             |                |                |                |
|         0x03         | Prints a line                    | A pointer to the line |             |             |                |                |                |
