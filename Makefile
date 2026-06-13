CC=clang
LD=ld.lld
ASM=nasm

CFLAGS=-target i686-elf -ffreestanding -O2 -Wall -Wextra
ASMFLAGS=-f elf32
LDFLAGS=-T linker.ld

C_SRC=$(wildcard src/*.c)
ASM_SRC=$(wildcard src/*.asm)

C_OBJ=$(patsubst src/%.c,build/%.o,$(C_SRC))
ASM_OBJ=$(patsubst src/%.asm,build/%.o,$(ASM_SRC))

OBJ=$(C_OBJ) $(ASM_OBJ)

KERNEL=build/kernel.elf

all: nanoos.iso

build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.asm
	mkdir -p build
	$(ASM) $(ASMFLAGS) $< -o $@

$(KERNEL): $(OBJ)
	$(LD) -T linker.ld $(OBJ) -o $(KERNEL)

nanoos.iso: $(KERNEL)
	mkdir -p iso/boot/grub
	cp $(KERNEL) iso/boot/kernel.elf
	cp grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o nanoos.iso iso/

run: nanoos.iso
	qemu-system-x86_64 -cdrom nanoos.iso

clean:
	rm -rf build iso nanoos.iso

.PHONY: all run clean