
SRCs=zelda_loader.S
OBJ=zelda_loader.o
BIN=zelda_loader.bin

$(BIN): $(SRCs)
	@echo "[AS] $<"
	@as -32 -o $(OBJ) $(SRCs)
	@echo "[Imaging] $(BIN)"
	@cp $(OBJ) $(BIN)
	@objcopy $(BIN) -O binary

clean:
	@echo "[cleaning] $(OBJ) $(BIN)"
	@rm -f $(OBJ) $(BIN)
run:$(BIN)
	@#qemu-system-x86_64 -serial tcp::4444,server -monitor null -nographic -vnc :100 -drive file=zelda_loader.bin,if=ide
	@qemu-system-x86_64  -monitor null -nographic -vnc :100 -drive file=zelda_loader.bin,if=ide
