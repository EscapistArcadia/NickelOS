# underlying architecture specification
ARCH := x86_64

# EFI firmware directory specification
GNU_EFI_DIR := ../gnu-efi
GNU_EFI_ARCH_DIR := $(GNU_EFI_DIR)/$(ARCH)
GNU_EFI_INCLUDE_DIR := $(GNU_EFI_DIR)/inc
GNU_EFI_LIB_DIR := $(GNU_EFI_ARCH_DIR)/lib
GNU_EFI_STARTSRC_DIR := $(GNU_EFI_DIR)/gnuefi
GNU_EFI_EXEC_DIR := $(GNU_EFI_ARCH_DIR)/gnuefi
GNU_EFI_LOADER := $(GNU_EFI_STARTSRC_DIR)/elf_$(ARCH)_efi.lds
GNU_EFI_CRT0 := $(GNU_EFI_EXEC_DIR)/crt0-efi-$(ARCH).o

# compiler specification
CC := gcc
LD := ld

EFI_DIR := ./efi
EFI_CFLAGS += -nostdlib -fno-builtin -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -I $(GNU_EFI_INCLUDE_DIR)
EFI_LDFLAGS += -shared -Bsymbolic -L $(GNU_EFI_LIB_DIR) -L $(GNU_EFI_EXEC_DIR) -T $(GNU_EFI_LOADER) $(GNU_EFI_CRT0)

EFI_SRCS += $(shell find $(EFI_DIR) -name '*.s') $(shell find $(EFI_DIR) -name '*.c')
EFI_OBJS += $(patsubst %.s,%.o,$(filter %.s,$(EFI_SRCS))) $(patsubst %.c,%.o,$(filter %.c,$(EFI_SRCS)))
EFI_SO := $(EFI_DIR)/bootx64.so
EFI_LOADER := $(EFI_DIR)/bootx64.efi

KERNEL_CFLAGS += -g -Wall -fno-builtin -fno-stack-protector -mno-red-zone -nostdlib -m64 $(KERNEL_INCLUDE)
KERNEL_LDFLAGS += -ffreestanding -nostdlib -static

KERNEL_DIR := ./kernel
KERNEL_LINKER := $(KERNEL_DIR)/linker.ld
KERNEL_IMAGE := ./Nickel.elf
KERNEL_BINARY := ./Nickel.bin

KERNEL_SRCS += $(shell find $(KERNEL_DIR) -name '*.s') $(shell find $(KERNEL_DIR) -name '*.c')
KERNEL_OBJS += $(patsubst %.s,%.o,$(filter %.s,$(KERNEL_SRCS))) $(patsubst %.c,%.o,$(filter %.c,$(KERNEL_SRCS)))
KERNEL_INCLUDE += -I ./include
KERNEL_INCLUDE += -I ./arch/$(ARCH)/include

FILE_SYSTEM_IMAGE := filesys.img

all: efi kernimg filesys

$(EFI_DIR)/%.o: $(EFI_DIR)/%.c
	$(CC) $(EFI_CFLAGS) -c $< -o $@

$(KERNEL_DIR)/%.o: $(KERNEL_DIR)/%.c
	$(CC) $(KERNEL_CFLAGS) -c $< -o $@

# uefi bootloader
# reference: https://wiki.osdev.org/GNU-EFI
efi: $(EFI_OBJS)
	$(LD) $(EFI_LDFLAGS) $(EFI_OBJS) -o $(EFI_SO) -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-$(ARCH) --subsystem=10 $(EFI_SO) $(EFI_LOADER)

kernimg: $(KERNEL_OBJS)
	$(CC) -T $(KERNEL_LINKER) -o $(KERNEL_IMAGE) $(KERNEL_LDFLAGS) $(KERNEL_OBJS)
	objcopy -O binary $(KERNEL_IMAGE) $(KERNEL_BINARY)

filesys:
	dd if=/dev/zero of=$(FILE_SYSTEM_IMAGE) bs=512 count=524288
	mkfs.fat -F 32 $(FILE_SYSTEM_IMAGE)
	mmd -i $(FILE_SYSTEM_IMAGE) ::/EFI
	mmd -i $(FILE_SYSTEM_IMAGE) ::/EFI/BOOT
	mcopy -i $(FILE_SYSTEM_IMAGE) $(EFI_LOADER) ::/EFI/BOOT/BOOTX64.EFI
	mmd -i $(FILE_SYSTEM_IMAGE) ::/KERNEL
	mcopy -i $(FILE_SYSTEM_IMAGE) $(KERNEL_IMAGE) ::/KERNEL/NICKEL.EXE

run:
	qemu-system-$(ARCH) -drive format=raw,file=$(FILE_SYSTEM_IMAGE) -bios /usr/share/ovmf/OVMF.fd -S -s

clean:
	rm -f $(EFI_OBJS) $(KERNEL_OBJS) $(EFI_SO) $(EFI_LOADER) $(FILE_SYSTEM_IMAGE) $(KERNEL_IMAGE) $(KERNEL_BINARY)
