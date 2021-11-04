#include "sdkconfig.h"

ENTRY(_boot)

MEMORY
{
#if defined(CONFIG_LINK_SCRIPT_ROM)
    ROM (rx) : ORIGIN = CONFIG_ROM_START_UP_ADDR, LENGTH = CONFIG_ROM_SIZE_MB * 1M
#endif
#if defined(CONFIG_LINK_SCRIPT_RAM)
    RAM (rwx): ORIGIN = CONFIG_RAM_START_UP_ADDR, LENGTH = CONFIG_RAM_SIZE_MB * 1M
#endif
}


SECTIONS
{

    .text :
	{
		KEEP(*(.StartUp))
        KEEP(*(.vectors))
		KEEP(*(.text.boot))
		*(.text)
        . = ALIGN(8);
	}> ROM


    .rodata :
    {
        *(.rodata .rodata.* .gnu.linkonce.r.*)

        . = ALIGN(8);
        _shell_command_start = .;
        KEEP (*(shellCommand))
        _shell_command_end = .;        
    } > ROM

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
        HeapBase = .;
        _heap_start = .;
        . = . + CONFIG_HEAP_SIZE;
        _heap_end = .;
        HeapLimit = .;
    } > RAM


    . = CONFIG_STACK_TOP_ADDR ; /* stack memory */
    stack_top = .;


}
__bss_size = (__bss_end - __bss_start)>>3;