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

APP_SRC=$(wildcard apps/*.c)
APP_ELF=$(patsubst apps/%.c,build/apps/%.elf,$(APP_SRC))

APP_CFLAGS=-target i686-elf -ffreestanding -O2 -Wall -Wextra
APP_LDFLAGS=-T app.ld

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

build/apps/%.elf: apps/%.c
	mkdir -p build/apps
	$(CC) $(APP_CFLAGS) -c $< -o build/apps/$*.o
	$(LD) $(APP_LDFLAGS) build/apps/$*.o -o $@

apps: $(APP_ELF)

$(DISK): $(KERNEL) apps
	rm -f $(DISK)

	dd if=/dev/zero of=$(DISK) bs=1M count=64

	parted $(DISK) --script mklabel msdos
	parted $(DISK) --script mkpart primary fat32 1MiB 100%

	LOOP=$$(sudo losetup --show -f -P $(DISK)); \
	sudo mkfs.fat -F 32 $${LOOP}p1; \
	mkdir -p $(MOUNT); \
	sudo mount $${LOOP}p1 $(MOUNT); \
	sudo mkdir -p $(MOUNT)/boot/grub; \
	sudo mkdir -p $(MOUNT)/apps; \
	sudo cp $(KERNEL) $(MOUNT)/boot/kernel.elf; \
	sudo cp grub.cfg $(MOUNT)/boot/grub/grub.cfg; \
	sudo cp build/apps/*.elf $(MOUNT)/apps/ || true; \
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

.PHONY: all run clean apps