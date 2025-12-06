KERNEL_PHYS_START = 1M;
KERNEL_STACK_SIZE = 0x4000;

ENTRY(_start)

SECTIONS {
    . = KERNEL_PHYS_START;

    .text : ALIGN(4K) {
        KEEP(*(.multiboot))
        *(.text*)
    }

    .rodata : ALIGN(4K) {
        *(.rodata*)
    }

    .data : ALIGN(4K) {
        *(.data*)
    }

    .bss : ALIGN(4K) {
        *(COMMON)
        *(.bss*)

        . = ALIGN(16);
        . += KERNEL_STACK_SIZE;
    }
}
