###############################################################################
#	makefile
#	 by Alex Chadwick
#
#	A makefile script for generation of raspberry pi kernel images.
###############################################################################

# The toolchain to use. arm-none-eabi works, but there does exist
# arm-bcm2708-linux-gnueabi.
ARMGNU ?= arm-none-eabi

# The intermediate directory for compiled object files.
BUILD = build/

# The directory in which source files are stored.
SOURCE = source/

# The name of the output file to generate.
TARGET = kernel.img

# The name of the assembler listing file to generate.
LIST = kernel.list

# The name of the map file to generate.
MAP = kernel.map

# The name of the linker script to use.
LINKER = kernel.ld

# The names of libraries to use.
LIBRARIES := csud

# The names of fs.img.
FS_IMG := fs.img

# CFLAGS := -fno-pic -static -fno-builtin -fno-strict-aliasing -Wall -MD
# CFLAGS += -ggdb -Werror -fno-omit-frame-pointer -nostdinc -nostdlib
# CFLAGS += -fno-stack-protector
CFLAGS := -fno-pic -static -Wno-packed-bitfield-compat
CFLAGS += -fno-builtin -fno-strict-aliasing -fshort-wchar -Wall -MD
CFLAGS += -ggdb -Werror -fno-omit-frame-pointer -fno-stack-protector

# CFLAGS += -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
# uncomment this line for rpi2
CFLAGS += -DRPI2 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7-a
# CFLAGS += -mtune=cortex-a7
CFLAGS += -O0

# -fno-short-enums: stops error msg
# uses variable-size enums yet the output is to use 32-bit enums;
# use of enum values across objects may fail
CFLAGS += -fno-short-enums

CFLAGS += -I include

CC := $(ARMGNU)-gcc

# The names of all object files that must be generated. Deduced from the
# assembly code files in source.
OBJECTS := $(patsubst $(SOURCE)%.S,$(BUILD)%.o,$(wildcard $(SOURCE)*.S))
OBJECTS += $(patsubst $(SOURCE)%.c,$(BUILD)%.o,$(wildcard $(SOURCE)*.c))
OBJECTS += $(BUILD)ramdisk $(BUILD)font

# Rule to make everything.
all: $(TARGET) $(LIST)

# Rule to remake everything. Does not include clean.
#rebuild: all

# Rule to make the listing file.
$(LIST) : $(BUILD)output.elf
	$(ARMGNU)-objdump -d $(BUILD)output.elf > $(LIST)

# Rule to make the image file.
$(TARGET) : $(BUILD)output.elf
	$(ARMGNU)-objcopy $(BUILD)output.elf -O binary $(TARGET)

# Rule to make the elf file.
# LIBOPTS := -L"/opt/local/arm-none-eabi/lib/" -lc
LIBOPTS := -L. $(patsubst %,-l %,$(LIBRARIES))
# this is for undefined reference to `__aeabi_idiv' error
LIBOPTS += -L"/opt/local/lib/gcc/arm-none-eabi/5.1.0/" -lgcc

$(BUILD)output.elf : $(LINKER) $(OBJECTS) $(BINARIES)
	$(ARMGNU)-ld -T $(LINKER) -o $(BUILD)output.elf -Map $(MAP) \
	             --no-undefined $(OBJECTS) $(LIBOPTS)

# Rule to make the object files.
$(BUILD)font: $(SOURCE)font1.bin
	$(ARMGNU)-objcopy -I binary -O elf32-littlearm -B arm $^ $@

$(BUILD)ramdisk: $(FS_IMG)
	$(ARMGNU)-objcopy -I binary -O elf32-littlearm -B arm $^ $@

# $(BUILD)%.o: $(SOURCE)%.s $(BUILD)
# 	$(ARMGNU)-as -I $(SOURCE) $< -o $@

$(BUILD)initcode.o: $(SOURCE)initcode.S $(BUILD)
	$(CC) -I include -c $< -o $(BUILD)initcode.out
	$(ARMGNU)-ld -N -s -e start -Ttext 0  -o $@  $(BUILD)initcode.out

$(BUILD)%.o: $(SOURCE)%.S $(BUILD)
	$(CC) -I include -c $<  -o $@

$(BUILD)%.o: $(SOURCE)%.c $(BUILD)
	$(CC) -c $(CFLAGS) $<  -o $@

$(BUILD):
	mkdir -p $@

# Rule to make fs.
$(FS_IMG): fs

fs :
	make -C uprogs

# Rule to clean files.
clean_fs:
	make clean -C uprogs

clean :
	-rm -rf $(BUILD)
	-rm -f $(TARGET)
	-rm -f $(LIST)
	-rm -f $(MAP)
