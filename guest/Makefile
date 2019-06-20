
C_FILES = $(foreach item,.,$(wildcard $(item)/*.c))
C_OBJS = $(patsubst %.c,%.o,$(C_FILES))

AS_FILES = $(foreach item, .,$(wildcard $(item)/*.S))
AS_OBJS = $(patsubst %.S,%.o,$(AS_FILES))

ASPARAMS = -m64 -mno-red-zone
CCPARAMS = -m64 -O0 -g -ffreestanding -mno-red-zone -nostdlib -fno-builtin -fno-exceptions -Werror -Wall -Wstrict-prototypes
LDPARAMS = -m elf_x86_64 -static

GUEST_ELF = guest64.elf
GUEST_IMG = guest64.img
GUEST_MAP = guest64.map
%.o: %.S
	@echo "[AS] $<"
	@gcc $(CCPARAMS) -I. -o $@ -c $<

%.o: %.c
	@echo "[CC] $<"
	@gcc $(CCPARAMS) -I. -o $@ -c $<

$(GUEST_IMG):$(GUEST_ELF)
	@echo "[CT] $@"
	@cp $< $@
	@objcopy  $@ -O binary

$(GUEST_ELF):$(AS_OBJS) $(C_OBJS)
	@echo "[LD] $@"
	@ld $(LDPARAMS) -Map=$(GUEST_MAP) -T guest64.ld -o $(GUEST_ELF) $(AS_OBJS) $(C_OBJS)

clean:
	@echo "[Cleaning] guest64"
	@rm -f *.o $(GUEST_ELF) $(GUEST_IMG) $(GUEST_MAP)
