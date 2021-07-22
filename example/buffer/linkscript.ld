
ENTRY(_boot)

MEMORY
{
    ROM (rx) : ORIGIN = 0x80100000, LENGTH = 1M
    RAM (rwx): ORIGIN = 0x81000000, LENGTH = 32M
}


SECTIONS
{

    .text :
	{
		KEEP(*(.StartUp))
        KEEP(*(.vectors))
		KEEP(*(.text.boot))
		*(.text)
        *(.rodata*)
        . = ALIGN(8);
        _text_end = .;
	}> ROM


     .init :
    {
        KEEP (*(SORT_NONE(.init)))
    }> ROM

    .fini :
    {
        KEEP (*(SORT_NONE(.fini)))
    }> ROM

    .eh_frame :
    {
        KEEP (*(.eh_frame))
    }> ROM

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    }> ROM

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    }> ROM

    .fini_array :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array ))
        PROVIDE_HIDDEN (__fini_array_end = .);
    }> ROM

    .dtors :
    {
        KEEP (*crtbegin.o(.dtors))
        KEEP (*crtbegin?.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
        . = ALIGN(16);
        _rom_end = .;
    }> ROM

	.data :AT(ADDR(.dtors) + SIZEOF(.dtors))
	{
        __data_start = .;
		*(.data)
        *(.data .data.* .gnu.linkonce.d.*)
        SORT(CONSTRUCTORS)
        . = ALIGN(4096); /* align to page size */
        __data_end = .;
	}> RAM



	.bss :
	{
        . = ALIGN(8);
        __bss_start__ = .;
        __bss_start = __bss_start__;
		bss = .;
		*(.bss)
        . = ALIGN(8);
        __bss_end__ = .;
        __bss_end = __bss_end__;
	}> RAM

    .heap (NOLOAD):
    {
        . = ALIGN(64);
        __end__ = .;
        PROVIDE(end = .);
        . = . + 0x1000;
    } > RAM


    . = 0x82000000 ; /* stack memory */
    stack_top = .;


}
__bss_size = (__bss_end - __bss_start)>>3;