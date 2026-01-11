/* Quin OS Kernel Linker Script */

OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

/* Higher half kernel */
PHDRS
{
    text    PT_LOAD FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata  PT_LOAD FLAGS((1 << 2)) ;            /* Read only */
    data    PT_LOAD FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
    dynamic PT_DYNAMIC FLAGS((1 << 1) | (1 << 2)) ; /* Dynamic */
}

SECTIONS
{
    /* Limine requests kernel to be loaded at -2GiB */
    . = 0xFFFFFFFF80000000;

    .text : {
        *(.text .text.*)
    } :text

    /* Move to the next page */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data : {
        *(.data .data.*)
        
        /* Limine requests */
        . = ALIGN(8);
        *(.limine_requests_start_marker)
        *(.limine_requests)
        *(.limine_requests_end_marker)
    } :data

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /* Kernel stack */
    . = ALIGN(16);
    . += 0x10000; /* 64KB stack */
    _stack_top = .;

    /DISCARD/ : {
        *(.eh_frame)
        *(.note .note.*)
        *(.comment)
    }
}
