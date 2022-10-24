OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(kmain)

PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ;
    rodata  PT_LOAD    FLAGS((1 << 2)) ;
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ;
}

SECTIONS
{
    . = 0xffffffff80000000;
    
    text_start_address = .;
    .text : {
        *(.text .text.*)
    } :text
    text_end_address = .;

    . += CONSTANT(MAXPAGESIZE);

    rodata_start_address = .;
    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    .symbol_table : {
        *(.symbol_table)
    } :rodata

    .limine_reqs : {
        KEEP(*(.limine_reqs))
    } :rodata
    rodata_end_address = .;

    . += CONSTANT(MAXPAGESIZE);

    data_start_address = .;
    .data : {
        *(.data .data.*)
    } :data

    .bss : {
        *(COMMON)
        *(.bss .bss.*)
    } :data
    data_end_address = .;

    /DISCARD/ : {
        *(.eh_frame)
        *(.note .note.*)
    }
}
