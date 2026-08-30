CC=clang
AS=nasm
LD=ld.lld
QEMU=qemu-system-i386 -m 1024

SRC_DIR=./src
APP_DIR=./apps
MODULE_DIR=./modules

BUILD_DIR=./build
APP_BUILD_DIR=$(BUILD_DIR)/apps
MODULE_BUILD_DIR=$(BUILD_DIR)/modules
MOUNT_DIR=$(BUILD_DIR)/mnt

LIBRARY_DIR := ./library
LIBRARY_SRC_DIR := $(LIBRARY_DIR)/src
LIBRARY_INCLUDE_DIR := $(LIBRARY_DIR)/include
LIBRARY_BUILD_DIR := $(BUILD_DIR)/library

LIBRARY_NAME := libnano.a
LIBRARY := $(LIBRARY_BUILD_DIR)/$(LIBRARY_NAME)

LIBRARY_C_SRC := $(shell find $(LIBRARY_SRC_DIR) -type f -name "*.c")
LIBRARY_OBJ := $(patsubst $(LIBRARY_SRC_DIR)/%.c,$(LIBRARY_BUILD_DIR)/%.o,$(LIBRARY_C_SRC))

DISK_IMG=$(BUILD_DIR)/nanoos.img
KERNEL_ELF=$(BUILD_DIR)/kernel.elf

KERNEL_LD=linker.ld
APP_LD=app.ld
MODULE_LD=module.ld

CFLAGS=-target i386 -m32 -ffreestanding -O2 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -Wextra -c -I$(LIBRARY_INCLUDE_DIR) -Iinclude
ASFLAGS=-f elf32

LDFLAGS=-m elf_i386 -T $(KERNEL_LD)
APP_LDFLAGS=-m elf_i386 -T $(APP_LD)
MODULE_LDFLAGS=-m elf_i386 -T $(MODULE_LD)

C_SRC := $(shell find $(SRC_DIR) -type f -name "*.c")
ASM_SRC := $(shell find $(SRC_DIR) -type f -name "*.asm")

C_OBJ := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/src/%.o,$(C_SRC))
ASM_OBJ := $(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/src/%.o,$(ASM_SRC))

APP_NAMES := $(notdir $(wildcard $(APP_DIR)/*))
APP_ELFS := $(foreach app,$(APP_NAMES),$(APP_BUILD_DIR)/$(app).elf)

MODULE_NAMES := $(notdir $(wildcard $(MODULE_DIR)/*))
MODULE_ELFS := $(foreach mod,$(MODULE_NAMES),$(MODULE_BUILD_DIR)/$(mod).elf)

AR=ar
ARFLAGS=rcs

all: apps modules image

.PHONY: library

library: $(LIBRARY)

$(LIBRARY): $(LIBRARY_OBJ)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^

$(LIBRARY_BUILD_DIR)/%.o: $(LIBRARY_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

kernel: $(KERNEL_ELF)

apps: $(LIBRARY) $(APP_ELFS)

modules: $(MODULE_ELFS)

$(KERNEL_ELF): $(C_OBJ) $(ASM_OBJ) $(LIBRARY)
	@mkdir -p $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/src/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@


$(BUILD_DIR)/src/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

define APP_template

APP_$(1)_C_SRCS := $$(wildcard $(APP_DIR)/$(1)/*.c)
APP_$(1)_ASM_SRCS := $$(wildcard $(APP_DIR)/$(1)/*.asm)

APP_$(1)_OBJS := \
    $$(patsubst $(APP_DIR)/%.c,$(APP_BUILD_DIR)/%.o,$$(APP_$(1)_C_SRCS)) \
    $$(patsubst $(APP_DIR)/%.asm,$(APP_BUILD_DIR)/%.o,$$(APP_$(1)_ASM_SRCS))

$(APP_BUILD_DIR)/$(1).elf: $$(APP_$(1)_OBJS) $(LIBRARY)
	@mkdir -p $$(@D)
	$(LD) $(APP_LDFLAGS) -o $$@ $$^

endef

$(foreach app,$(APP_NAMES),$(eval $(call APP_template,$(app))))

define MODULE_template

MODULE_$(1)_C_SRCS := $$(wildcard $(MODULE_DIR)/$(1)/*.c)
MODULE_$(1)_ASM_SRCS := $$(wildcard $(MODULE_DIR)/$(1)/*.asm)

MODULE_$(1)_OBJS := \
    $$(patsubst $(MODULE_DIR)/%.c,$(MODULE_BUILD_DIR)/%.o,$$(MODULE_$(1)_C_SRCS)) \
    $$(patsubst $(MODULE_DIR)/%.asm,$(MODULE_BUILD_DIR)/%.o,$$(MODULE_$(1)_ASM_SRCS))

$(MODULE_BUILD_DIR)/$(1).elf: $$(MODULE_$(1)_OBJS)
	@mkdir -p $$(@D)
	$(LD) $(MODULE_LDFLAGS) -o $$@ $$^

endef

$(foreach mod,$(MODULE_NAMES),$(eval $(call MODULE_template,$(mod))))

$(MODULE_BUILD_DIR)/%.o: $(MODULE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

$(MODULE_BUILD_DIR)/%.o: $(MODULE_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@
	
$(APP_BUILD_DIR)/%.o: $(APP_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

$(APP_BUILD_DIR)/%.o: $(APP_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

image: kernel apps modules
	@echo "Creating bootable disk image..."

	mkdir -p $(BUILD_DIR)

	dd if=/dev/zero of=$(DISK_IMG) bs=1M count=64

	parted -s $(DISK_IMG) mklabel msdos
	parted -s $(DISK_IMG) mkpart primary fat32 1MiB 100%
	parted -s $(DISK_IMG) set 1 boot on

	LOOP=$$(sudo losetup --find --show --partscan $(DISK_IMG)); \
		echo "Loop device: $$LOOP"; \
		sudo mkfs.vfat -F32 $${LOOP}p1; \
		mkdir -p $(MOUNT_DIR); \
		sudo mount $${LOOP}p1 $(MOUNT_DIR); \
		sudo mkdir -p $(MOUNT_DIR)/boot/grub; \
		sudo mkdir -p $(MOUNT_DIR)/bin; \
		sudo mkdir -p $(MOUNT_DIR)/modules; \
		sudo cp $(KERNEL_ELF) $(MOUNT_DIR)/boot/kernel.elf; \
		sudo cp $(APP_ELFS) $(MOUNT_DIR)/bin/; \
		sudo cp $(MODULE_ELFS) $(MOUNT_DIR)/modules/; \
		echo 'set timeout=0' | sudo tee $(MOUNT_DIR)/boot/grub/grub.cfg >/dev/null; \
		echo 'set default=0' | sudo tee -a $(MOUNT_DIR)/boot/grub/grub.cfg >/dev/null; \
		echo 'menuentry "NanoOS" {' | sudo tee -a $(MOUNT_DIR)/boot/grub/grub.cfg >/dev/null; \
		echo '    multiboot /boot/kernel.elf' | sudo tee -a $(MOUNT_DIR)/boot/grub/grub.cfg >/dev/null; \
		echo '    boot' | sudo tee -a $(MOUNT_DIR)/boot/grub/grub.cfg >/dev/null; \
		echo '}' | sudo tee -a $(MOUNT_DIR)/boot/grub/grub.cfg >/dev/null; \
		sudo grub-install \
			--target=i386-pc \
			--boot-directory=$(MOUNT_DIR)/boot \
			--modules="part_msdos fat multiboot" \
			--locales= \
			--recheck \
			$$LOOP; \
		sudo umount $(MOUNT_DIR); \
		sudo losetup -d $$LOOP

run: image
	$(QEMU) -drive format=raw,file=$(DISK_IMG)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all kernel apps modules image run clean