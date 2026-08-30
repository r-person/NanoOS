# NanoOS

NanoOS is a 32-bits x86 OS built with the goal of having a small core and the 
possibility of expanding it. 

# How to run it?

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

Currently (as of Beta v1.0.0-rc1) the OS supports the next features:

- Booting using GRUB into Protected Mode.

- A basic text-mode shell with basic commands.

- Informing the user about CPU exception (except double faults) and halting 
without crashing. In case of a double fault the kernel resets the computer. 
In case of IRQ1 the kernel gets the keyboard input. In case of IRQ0 the timer 
ticks counter is increased, and in case of IRQ2 - IRQ15 nothing happens.

- The OS is also capable of handling syscalls (int 0x84), internal syscalls 
(int 0x81), internal UI syscalls (int 0x82), and internal filesystem 
interrupts (int 0x83). 

- Handling basic FAT32 actions. 

- Paging (higher half kernel).

- Loading ELF apps and running them at ring 3 with isolation from the kernel. 
The system is currently single-tasking,

- Basic timer actions. 

# New features

Added support for ELF apps (using the 'run' command). The paging was updated 
so the kernel is now in the higher half. The int 0x84 (in past versions int 
0x80) syscalls were extended. They were moved to interrupt 0x84 because 
comptiability for Linux apps is planned.

# Shell commands

## version

Prints the current NanoOS version. 

## reboot

Reboots the computer.

## echo

Prints into the screen the text written after the command.

## filenum

Prints the count of files and directories in the subdirectory specified after 
the command (or the root directory if not specified any subdirectory). This 
includes "." and "..".

## dir

Prints all the directories and files in the subdirectory specified after the 
command (or the root directory if not specified any subdirectory).

## run

Runs the ELF file specified after the run command. The ELF must be in the /bin 
folder. No need to add the .elf extension (if added to the command, the OS 
would attempt to load the wrong file).

# Warning

This version is a release candidate. It means that this is not ready yet. There
are still bugs that need to be solved. 

# FAQ

## How to install apps on it?

You can either add the source code to ./apps (and then the makefile will 
compile it automatically) or add the compiled ELF file of the app to the /bin 
directory in the .img file.

## Does libc exist for NanoOS?

No. The OS is too small to support libc as of now, so instead there are custom 
libraries at ./include. Take in mind that the functions in the custom 
libraries that have the same name as libc functions might have diffrent 
behaviour than the libc version of the function. 

## What else should I take in mind in addition to the custom libraries while 
writing NanoOS apps?

The stack is set up when the app is runned. In additional, it is recommended 
for the app use the app.ld script. If you have any questions related to 
writing apps that are not answered here, you can view the source code of the 
apps in ./apps. 

## Are the system calls guarenteed to exist?

The system calls (including both internal syscalls and int 0x80 and int 0x84) 
are guarenteed to exist in every new NanoKernel version with backwards 
comptiability without being removed once they are added, unless there is a 
very good reason. If a syscall is removed the syscall's number will be unused. 

## Can I run it on real hardware?

The OS was tested only on QEMU, but it doesn't mean it can't run on real 
hardware. Anyways, as of now, it is not recommended because NanoOS is 
programmed while assuming it is runned on QEMU, and because it is still not 
stable enough.

## What are the minimal requirements of it?

You should make sure that there are:

- A x86 CPU that supports protected mode.

- A legacy ATA (PATA) controller. 

- At least 64MiB hard disk memory and 32MiB RAM. 

## How to mod the OS?

To mod it you can simply edit the source code and then compile as explained 
before. If you want to replace the UI you need to take in mind that 
__uisystemcall is the entery of int 0x82 and you need to handle all the UI 
system calls. If you want to replace the filesystem handler you need to take 
in mind that __filesystemsystemcall is the entery of int 0x83 and you need to 
handle all the filesystem system calls.

## Can I do pull requests?

No. I don't want to review pull requests. If you want to make changes, add new 
features or patch bugs make your own fork. 

## Can I report bugs?

No. 

## Can I contribute in any other way?

Yes. An OS is useless without anything to run on it. You can make apps or 
libraries for it. 

# Syscalls

System get up to 3 arguments in the registers and return up to 3 output values 
in the registers. If there are less than 3 output values the unused registers 
stay unchanged. The default handlers do nothing if an unrecognized syscall is 
called. The arguments are unsigned unless written something else. As specified 
before, the system calls (both user syscalls and internal syscalls) are 
guarenteed to exist with their visible result to the calling process in the 
same form they were added in, unless written elsewards. 

## Int 0x81

Int 0x81 is the internal kernel syscalls.

| Syscall number (EAX) |           goal           | arg 1 (EBX) |     arg 2 (ECX)     |      arg 3 (EDX)      | output 1 (EAX) | output 2 (EBX) | output 3 (ECX) |
|----------------------|--------------------------|-------------|---------------------|-----------------------|----------------|----------------|----------------|
|         0x00         | Reboot request           |             |                     |                       |                |                |                |
|         0x01         | Sectors reading request  | LBA adress  | Amount of sectors   | Pointer to the buffer |                |                |                |
|         0x02         | IDT gate request         | IDT number  | Pointer to the gate |                       |                |                |                |

## Int 0x82

Int 0x82 is the internal UI syscalls.

| Syscall number (EAX) |                   goal                   |       arg 1 (EBX)       | arg 2 (ECX) | arg 3 (EDX) |                   output 1 (EAX)                   |  output 2 (EBX)  | output 3 (ECX) |
|----------------------|------------------------------------------|-------------------------|-------------|-------------|----------------------------------------------------|------------------|----------------|
|         0x00         | Setting up the UI                        |                         |             |             |                                                    |                  |                |
|         0x01         | Getting an input character               | The char                |             |             |                                                    |                  |                |
|         0x02         | Prints a string                          | A pointer to the string |             |             |                                                    |                  |                |
|         0x03         | Reads from STDIN                         |                         |             |             | The character (or 0x00 if no characters available) |                  |                |
|         0x04         | Clears the screen                        |                         |             |             |                                                    |                  |                |

## Int 0x83

Int 0x83 is the internal filesystem syscalls.

| Syscall number (EAX) |                            goal                            |            arg 1 (EBX)            | arg 2 (ECX) |                                                                       arg 3 (EDX)                                                                       |       output 1 (EAX)       | output 2 (EBX) | output 3 (ECX) |
|----------------------|------------------------------------------------------------|-----------------------------------|-------------|---------------------------------------------------------------------------------------------------------------------------------------------------------|----------------------------|----------------|----------------|
|         0x00         | Sets up                                                    |                                   |             |                                                                                                                                                         |                            |                |                |
|         0x01         | Returns the amount all the bytes that a file takes         |                                   |             |                                                                                                                                                         | The total amount of bytes  |                |                |
|         0x02         | Reads a file                                               | The file's first cluster          | The buffer  | Pointer to the extra arguments, one after another: 1. 4 bytes with the amount of the clusters to load. 2. 4 bytes with the amount of clusters to skip.  |                            |                |                |
|         0x03         | Returns the number of files at the directory               | A pointer to the path of the dir  |             |                                                                                                                                                         | The total number           |                |                |
|         0x04         | Prints all the files and directories at the directory      | A pointer to the path of the dir  |             |                                                                                                                                                         |                            |                |                |
|         0x05         | Finds the first cluster of a file                          | A pointer to the filepath         |             |                                                                                                                                                         | The file's first cluster   |                |                |


## Int 0x84

Int 0x84 is the kernel syscalls. This is the only visible system calls 
interrupt visible to apps (except the int 0x80, that will be added in one of 
the next versions). User apps should not assume that the error codes specified 
here are the only ones, as it is possible that more exit codes will be added. 

| Syscall number (EAX) |              goal              |        arg 1 (EBX)        |       arg 2 (ECX)       |   arg 3 (EDX)   |                                                 output 1 (EAX)                                                 |     output 2 (EBX)     |     output 3 (ECX)     | Notes |
|----------------------|--------------------------------|---------------------------|-------------------------|-----------------|----------------------------------------------------------------------------------------------------------------|------------------------|------------------------|-------|
|         0x00         | Get the NanoKernel version     |                           |                         |                 | Major version number.                                                                                          | Second version number. | Third version number.  |       |
|         0x01         | Writes to a file               | File descriptor           | A pointer to the string | String length   | Amount of bytes written or a negative exit code (-0x01 = permission denied, -0x02 = invaild pointer). Signed.  |                        |                        |       |
|         0x02         | Exists the program             | Exit code                 |                         |                 |                                                                                                                |                        |                        |       |
|         0x03         | Sleeps                         | The amount of miliseconds |                         |                 |                                                                                                                |                        |                        |       |
|         0x04         | Returns the system time in MS  |                           |                         |                 | The system time.                                                                                               |                        |                        |       |
|         0x05         | Reads characters from a file   | File descriptor           | A pointer to the buffer | Amount of bytes | Amount of bytes readen or a negative exit code (-0x01 - permission denied, -0x02 = invaild pointer). Signed.   |                        |                        |       |
