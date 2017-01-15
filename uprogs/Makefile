# Cross-compiling (e.g., on Mac OS X)
TOOLPREFIX = arm-none-eabi-
# Using native tools (e.g., on X86 Linux)
# TOOLPREFIX =


# The intermediate directory for compiled object files.
BUILD = build/

CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)as
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

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

CFLAGS += -I ../include

ASFLAGS := -I ../include

LIBGCC := -L"/opt/local/lib/gcc/arm-none-eabi/5.1.0/" -lgcc

MKFS = ./mkfs
FS_IMAGE = ../fs.img

ULIB = ulib.o usys.o printf.o umalloc.o

UPROGS=\
	_cat\
	_cpu\
	_echo\
	_forktest\
	_grep\
	_init\
	_kill\
	_ln\
	_ls\
	_mkdir\
	_print_a_lot\
	_rm\
	_sh\
	_smptest\
	_stressfs\
	_usertests\
	_wc\
	_zombie\


all: $(FS_IMAGE)

_%: %.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^  -L ../ $(LIBGCC)
	$(OBJDUMP) -S $@ > $*.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $*.sym

_forktest: forktest.o $(ULIB)
	# forktest has less library code linked in - needs to be small
	# in order to be able to max out the proc table.
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o _forktest forktest.o ulib.o usys.o
	$(OBJDUMP) -S _forktest > forktest.asm

$(MKFS): mkfs.c ../include/fs.h
	gcc -Werror -Wall -iquote ../include -o $@ mkfs.c

$(FS_IMAGE): $(MKFS) README $(UPROGS)
	$(MKFS) $@ README $(UPROGS)
	$(OBJDUMP) -S usys.o > usys.asm

clean:
	rm -f *.o *.d *.asm *.sym $(FS_IMAGE) \
	.gdbinit \
	$(UPROGS)
