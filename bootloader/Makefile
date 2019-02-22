#Copyright (c) 2019 Jie Zheng
C_SRCS = $(wildcard *.c)
C_OBJS = $(patsubst %.c,%.o,$(C_SRCS))
ASM_SRCS=zelda_earlyload.S
ASM_OBJ=zelda_earlyload.o
ASM_BIN=zelda_earlyload.bin
C_BIN = zelda_postboot.bin
C_MAP = zelda_postboot.map
C_INC = zelda_postboot.inc
IMAGE = zelda_bootable.img
%.o:%.c
	@echo "[CC] $<"
	@gcc -g3 -O0 -m32 -c $< -o $@

%.o:%.S
	@echo "[AS] $<"
	@as -32 -o $@ $<
$(IMAGE):$(ASM_BIN) $(C_BIN)
	@echo "[Imaging] $(IMAGE)"
	@dd conv=notrunc if=$(ASM_BIN) of=$(IMAGE) status=none
	@dd conv=notrunc obs=512 if=$(C_BIN) of=$(IMAGE) seek=1 status=none

$(ASM_BIN): $(C_BIN) $(ASM_OBJ)
	@echo "[Imaging] $(ASM_BIN)"
	@cp $(ASM_OBJ) $(ASM_BIN)
	@objcopy $(ASM_BIN) -O binary
$(C_BIN): $(C_OBJS)
	@echo "[Imaging] $(C_BIN)"
	@ld -melf_i386 -Map=$(C_MAP) --oformat=binary -T linker.ld -o $(C_BIN) $(C_OBJS)
	@echo -n ".equ BOOTMAIN_SIZE, " > $(C_INC)
	@wc -c < $(C_BIN) >> $(C_INC)
clean:
	@echo "[cleaning] $(ASM_OBJ) $(ASM_BIN)"
	@rm -f $(ASM_OBJ) $(ASM_BIN) $(C_OBJS) $(C_BIN) $(C_MAP) $(C_INC) $(IMAGE)

run:$(IMAGE)
	@#qemu-system-x86_64 -serial tcp::4444,server -monitor null -nographic -vnc :100 -drive file=zelda_loader.bin,if=ide
	@qemu-system-x86_64  -monitor null -nographic -vnc :100 -drive file=$(IMAGE),if=ide  -gdb tcp::5070 
