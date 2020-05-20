#Copyright (c) 2019 Jie Zheng
#The common makefile for 64bit kernel

ifeq ($(ZELDA64),)
$(error 'please specify env variable ZELDA64')
endif

DEFS = -DZELDA64
CCPARAMS = -m64 -O0 -g -ffreestanding -mno-red-zone -nostdlib -fno-builtin -fno-exceptions -Werror -Wall -Wstrict-prototypes
ASPARAMS = -m64 -mno-red-zone
LDPARAMS = -m elf_x86_64 -static

ELF = Zelda64.elf
BIN = Zelda64.bin
MAP = Zelda64.map
KERNEL_INC = kernel64.inc

C_FILES = $(foreach item,$(DIRS),$(wildcard $(item)/*.c))
C_OBJS = $(patsubst %.c,%.o,$(C_FILES))

AS_FILES = $(foreach item,$(DIRS),$(wildcard $(item)/*.S))
AS_OBJS = $(patsubst %.S,%.o,$(AS_FILES))

IMAGE_DEPEND = $(C_OBJS) $(AS_OBJS)
%.o: %.c
	@echo "[CC] $<"
	@$(CC) $(CCPARAMS) $(DEFS) -DC_CONTEXT -I . -include zelda64_config.h  -o $@ -c $<

%.o: %.S
	@echo "[AS] $<"
	@$(CC) $(CCPARAMS) $(DEFS) -I . -include zelda64_config.h -o $@ -c $<

.PHONY:KERNEL_IMAGE
KERNEL_IMAGE: $(BIN)
	@echo -n ".equ BOOTMAIN_SIZE, " > $(KERNEL_INC)
	@wc -c < $(BIN) >> $(KERNEL_INC)

$(ELF): $(IMAGE_DEPEND)
	@echo "[LD] $@"
	@ld $(LDPARAMS) -Map=$(MAP) -T $(ZELDA64)/mk/kernel64.ld -o $(ELF) $(IMAGE_DEPEND)

$(BIN):$(ELF)
	@echo "[CT] $@"
	@cp $(ELF) $(BIN)
	@objcopy  $(BIN) -O binary

.PHONY:KERNEL_CLEAN
KERNEL_CLEAN:
	@echo "[Cleaning] kernel"
	@rm -f $(ELF) $(BIN) $(IMAGE_DEPEND) $(MAP) $(KERNEL_INC)
