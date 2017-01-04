OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

ENTRY_SVC_STACK_SIZE = 0x1000;

SECTIONS {
    /* Link the kernel at this address: "." means the current address */
    /* Must be equal to KERNLINK */
    . = 0x80000000;

    /*
    * First and formost we need the .init section, containing the code to
    * be run first. We allow room for the ATAGs and stack and conform to
    * the bootloader's expectation by putting this code at 0x8000.
    */
    .init 0x80000000 : AT(0x0) {
        build/entry.o(.vectors)
        build/entry.o(.text)
        build/start.o(.text .text.*)

        build/entry.o(.rodata .rodata.*)
        build/start.o(.rodata .rodata.*)

        build/entry.o(.data .data.*)
        build/start.o(.data .data.*)

        PROVIDE(_binary_initcode_start = .);
        build/initcode.o
        PROVIDE(_binary_initcode_end = .);

        PROVIDE(edata_entry = .);

        build/entry.o(.bss .bss.* COMMON)
        build/start.o(.bss .bss.* COMMON)

        /*define a stack for the entry*/
        . = ALIGN(0x1000);
        . += ENTRY_SVC_STACK_SIZE;

        PROVIDE (svc_stktop = .);

        /* define the kernel page table, must be 16K=0x4000 and 16K-aligned*/
        . = ALIGN(0x4000);
        PROVIDE (_kernel_pgtbl = .);
        . += 0x4000;

        /* we also need a user page table*/
        PROVIDE (_user_pgtbl = .);
        . += 0x1000;

        PROVIDE(end_entry = .);
    }

    /*
    * Next we put the rest of the code.
    */
    .text : {
        *.c.o(.text)
        *(.text .stub .text.*)
    }
    PROVIDE(etext = .);	/* Define the 'etext' symbol to this value */

    /*
    * read-only data
    */
    .rodata : {
        *(.rodata .rodata.*)
    }

    /* aligned the data to a (4K) page, so it can be assigned
     different protection than the code*/
    . = ALIGN(0x1000);
    PROVIDE (data_start = .);

    .data : {
        . = ALIGN(4);
        PROVIDE(font = .);
        build/font.o

        *(.data)
        *.c.o(*)
    }

    PROVIDE (edata = .);

    .bss : {
        *(.bss .bss.* COMMON)
    }

    .ramdisk : {
        . = ALIGN(4);
        PROVIDE(_binary_fs_img_start = .);
        build/ramdisk.o
        PROVIDE(_binary_fs_img_end = .);
    }

    . = ALIGN(0x1000);
    PROVIDE (end = .);

    /*
    * Finally comes everything else. A fun trick here is to put all other
    * sections into this section, which will be discarded by default.
    */
    /DISCARD/ : {
        *(.eh_frame .note.GNU-stack)
    }
}
