# Toolchain

CC      = arm-none-eabi-gcc
AS      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# Target
TARGET = main

# SELECT APPLICATION

# APP = src/bai1.c
# APP = src/bai2.c
# APP = src/bai3.c
 APP = src/bai4.c
#APP = src/main.c
# Flags

CFLAGS = -mcpu=cortex-m3 -mthumb -O0 -g \
         -Wall \
         -ffunction-sections \
         -fdata-sections \
         -DSTM32F10X_MD \
         -DUSE_STDPERIPH_DRIVER

LDFLAGS = -Tstm32f103c8t6.ld \
          -specs=nosys.specs \
          -Wl,--gc-sections \
          -lm
# Include paths
INCLUDES = -Isrc -Ilib

# Source files

# Application
APP_OBJ = $(APP:.c=.o)

# All library C files
LIB_SRC = $(wildcard lib/*.c)

# Library object files
LIB_OBJ = $(LIB_SRC:.c=.o)

# Startup
STARTUP_OBJ = startup_stm32f10x_md.o

# All objects
OBJ = $(APP_OBJ) $(LIB_OBJ) $(STARTUP_OBJ)

# Build
all: $(TARGET).elf $(TARGET).bin $(TARGET).hex

# Compile application
src/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile libraries
lib/%.o: lib/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile startup assembly
startup_stm32f10x_md.o: startup_stm32f10x_md.s
	$(AS) -mcpu=cortex-m3 -mthumb -c $< -o $@

# Link
$(TARGET).elf: $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@
	$(SIZE) $@

# Convert ELF -> BIN
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Convert ELF -> HEX
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

# Flash using ST-Link
flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x08000000

# Debug
debug: $(TARGET).elf
	gdb-multiarch $(TARGET).elf

# Clean
clean:
	rm -f src/*.o
	rm -f lib/*.o
	rm -f *.o
	rm -f *.elf
	rm -f *.bin
	rm -f *.hex

# Phony targets
.PHONY: all flash debug clean