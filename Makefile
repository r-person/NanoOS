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
DISK=nanoos.img
MOUNT=mnt

all: $(DISK)

build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.asm
	mkdir -p build
	$(ASM) $(ASMFLAGS) $< -o $@

$(KERNEL): $(OBJ)
	mkdir -p build
	$(LD) $(LDFLAGS) $(OBJ) -o $(KERNEL)

$(DISK): $(KERNEL)
	rm -f $(DISK)

	dd if=/dev/zero of=$(DISK) bs=1M count=64

	parted $(DISK) --script mklabel msdos
	parted $(DISK) --script mkpart primary fat32 1MiB 100%

	LOOP=$$(sudo losetup --show -f -P $(DISK)); \
	sudo mkfs.fat -F 32 $${LOOP}p1; \
	mkdir -p $(MOUNT); \
	sudo mount $${LOOP}p1 $(MOUNT); \
	sudo mkdir -p $(MOUNT)/boot/grub; \
	sudo cp $(KERNEL) $(MOUNT)/boot/kernel.elf; \
	sudo cp grub.cfg $(MOUNT)/boot/grub/grub.cfg; \
	sudo grub-install \
		--target=i386-pc \
		--boot-directory=$(MOUNT)/boot \
		--no-floppy \
		$${LOOP}; \
	sudo umount $(MOUNT); \
	sudo losetup -d $${LOOP}

run: $(DISK)
	qemu-system-i386 -drive format=raw,file=$(DISK)

clean:
	rm -rf build $(DISK) $(MOUNT)

.PHONY: all run clean