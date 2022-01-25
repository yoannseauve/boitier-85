CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
BIN = arm-none-eabi-objcopy
DUMP = arm-none-eabi-objdump
STL = st-flash

SRC_DIR = src

SRC := $(wildcard $(SRC_DIR)/*.c)
SRC += $(wildcard $(SRC_DIR)/*.s)
OBJ := $(foreach f, $(SRC:.c=.o), obj/$(notdir $(f:.s=.o)))

override CFLAGS += -mthumb -mcpu=cortex-m3 -MD -I ./src/Includes/StInclude/STM32F1xx/ -I ./src/Includes/StInclude/core/ -fdata-sections -ffunction-sections
override LDFLAGS += -T linker.ld --print-memory-usage --gc-sections
PRINTMAP =

.PHONY: app flash clean erase dump tags

all: app

debug: CFLAGS += -ggdb
debug: PRINTMAP += --print-map > linker_map
debug: |clean all

app: | make_dir app.bin

app.elf: $(OBJ) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(PRINTMAP)

app.bin: app.elf
	$(BIN) -O binary app.elf app.bin

obj/%.o: $(SRC_DIR)/%.c 
	$(CC) $(CFLAGS) -o $@ -c $<

obj/%.o: $(SRC_DIR)/%.s
	$(AS) -o $@ $< 

clean:
	rm -f *.elf *.bin linker_map
	rm -Rf obj

flash: app
	$(STL) write app.bin 0x8000000

dump:
	$(DUMP) --source --syms app.elf

make_dir:
	mkdir -p obj

erase:
	$(STL) erase

tags:
	ctags -R --language-force=c

-include obj/*.d
