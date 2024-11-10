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

all: efi
	$(info $(EFI_CFLAGS))

$(EFI_DIR)/%.o: $(EFI_DIR)/%.c
	$(CC) $(EFI_CFLAGS) -c $< -o $@

# uefi bootloader
efi: $(EFI_OBJS)
	$(LD) $(EFI_LDFLAGS) $(EFI_OBJS) -o $(EFI_SO) -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-$(ARCH) --subsystem=10 $(EFI_SO) $(EFI_LOADER)

clean:
	rm -f $(EFI_OBJS) $(EFI_SO) $(EFI_LOADER)
