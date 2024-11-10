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

EFI_SRCS += $(wildcard $(EFI_DIR)/*.c) $(wildcard $(EFI_DIR)/*.s)
EFI_OBJS += $(patsubst %.s,%.o,$(filter %.s,$(EFI_SRCS)))
EFI_OBJS += $(patsubst %.c,%.o,$(filter %.c,$(EFI_SRCS)))
EFI_SO := $(EFI_DIR)/bootx64.so
EFI_LOADER := $(EFI_DIR)/bootx64.efi

FILE_SYSTEM_IMAGE := filesys.img

all: efi filesys

$(EFI_DIR)/%.o: $(EFI_DIR)/%.c
	$(CC) $(EFI_CFLAGS) -c $< -o $@

# uefi bootloader
efi: $(EFI_OBJS)
	$(LD) $(EFI_LDFLAGS) $(EFI_OBJS) -o $(EFI_SO) -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-$(ARCH) --subsystem=10 $(EFI_SO) $(EFI_LOADER)

filesys:
	dd if=/dev/zero of=$(FILE_SYSTEM_IMAGE) bs=512 count=524288
	mkfs.fat -F 32 $(FILE_SYSTEM_IMAGE)
	mmd -i $(FILE_SYSTEM_IMAGE) ::/EFI
	mmd -i $(FILE_SYSTEM_IMAGE) ::/EFI/BOOT
	mcopy -i $(FILE_SYSTEM_IMAGE) $(EFI_LOADER) ::/EFI/BOOT/BOOTX64.EFI

run:
	qemu-system-$(ARCH) -drive format=raw,file=$(FILE_SYSTEM_IMAGE) -bios /usr/share/ovmf/OVMF.fd

clean:
	rm -f $(EFI_OBJS) $(EFI_SO) $(EFI_LOADER) $(FILE_SYSTEM_IMAGE)
