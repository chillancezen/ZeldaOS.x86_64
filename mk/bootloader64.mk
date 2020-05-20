#Copyright (c) 2019 Jie Zheng
ifeq ($(ZELDA64),)
$(error 'please specify env variable ZELDA64')
endif

BL_SRCS=bootloader64.S
BL_ELF =bootloader64.elf
BL_BIN =bootloader64.bin 

.PHONY:BOOTLOADER_IMAGE
BOOTLOADER_IMAGE: $(BL_BIN)

$(BL_ELF):$(ZELDA64)/bootloader/$(BL_SRCS)
	@echo "[AS] $<"
	@$(CC) -m32 -I . -o $@ -c $<

$(BL_BIN):$(BL_ELF)
	@echo "[BI] $@"
	@cp $< $@
	@objcopy $@ -O binary

BOOTLOADER_CLEAN:
	@echo "[Cleaning] bootloader"
	@rm -f $(BL_ELF) $(BL_BIN)
