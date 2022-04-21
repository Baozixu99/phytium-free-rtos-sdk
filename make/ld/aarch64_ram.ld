#include "sdkconfig.h"

MEMORY
{
    ROM (rx) : ORIGIN = CONFIG_ROM_START_UP_ADDR, LENGTH = CONFIG_ROM_SIZE_MB * 1M
    RAM (rwx): ORIGIN = CONFIG_RAM_START_UP_ADDR, LENGTH = CONFIG_RAM_SIZE_MB * 1M
}

ENTRY(_vector_table)

SECTIONS
{
    .text : {
        KEEP (*(.vectors))
        *(.boot)
        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)
        *(.plt)
        *(.gnu_warning)
        *(.gcc_execpt_table)
        *(.glue_7)
        *(.glue_7t)
        *(.ARM.extab)
        *(.gnu.linkonce.armextab.*)
    } > ROM

    .got : {
    *(.got)
    } > ROM

    .got1 : {
    *(.got1)
    } > ROM

    .got2 : {
    *(.got2)
    } > ROM

    .note.gnu.build-id : {
    KEEP (*(.note.gnu.build-id))
    } > ROM

    .init (ALIGN(64)): {
    KEEP (*(.init))
    } > ROM

    .fini (ALIGN(64)): {
    KEEP (*(.fini))
    } > ROM

    .interp : {
    __interp_start = .;
    KEEP (*(.interp))
    __interp_end = .;
    } > ROM

    .note-ABI-tag : {
    __note-ABI-tag_start = .;
    KEEP (*(.note-ABI-tag))
    __note-ABI-tag_end = .;
    } > ROM

    .preinit_array : {
    __preinit_array_start = .;
    KEEP (*(SORT(.preinit_array.*)))
    KEEP (*(.preinit_array))
    __preinit_array_end = .;
    } > ROM

    .init_array : {
    __init_array_start = .;
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    __init_array_end = .;
    } > ROM

    .fini_array : {
    __fini_array_start = .;
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array))
    __fini_array_end = .;
    } > ROM

    .rodata (ALIGN(64)): {
    __rodata_start = .;
    *(.rodata)
    *(.rodata.*)
    *(.gnu.linkonce.r.*)
    __rodata_end = .;
    } > ROM

    .le_shell  (ALIGN(64)):{
        . = ALIGN(4096);
        _shell_command_start = .;
        KEEP (*(shellCommand))
        _shell_command_end = .;
        . = ALIGN(4096); /* align to page size */
        _shell_code_end = .;
    }> ROM


    .rodata1 (ALIGN(64)): {
    __rodata1_start = .;
    *(.rodata1)
    *(.rodata1.*)
    __rodata1_end = .;
    } > ROM


    .gcc_except_table : {
    *(.gcc_except_table)
    . = ALIGN(16);
    __rom_end__ = .;
    } > ROM


    .data :AT(ADDR(.gcc_except_table) + SIZEOF(.gcc_except_table)){
        __data_start__ = .;
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d.*)
        *(.jcr)
        *(.got)
        *(.got.plt)
        . = ALIGN(4096); /* align to page size */
        __data_end__ = .;
    } > RAM

    .sbss (NOLOAD) : {
    __sbss_start__ = .;
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    __sbss_end__ = .;
    } > RAM

    .bss (NOLOAD) : {
    . = ALIGN(64);
    __bss_start__ = .;
    *(.bss)
    *(.bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN(64);
    __bss_end__ = .;
    } > RAM


    .heap (NOLOAD):{
        . = ALIGN(64);
        _heap = .;
        HeapBase = .;
        _heap_start = .;
        . = . + CONFIG_HEAP_SIZE* 1M;
        _heap_end = .;
        HeapLimit = .;
    } > RAM


    .stack (NOLOAD) : {
        . = ALIGN(64);
        _el1_stack_end = .;
        . += CONFIG_STACK_SIZE;
        __el1_stack = .;
    } > RAM

#ifdef  CONFIG_USE_OPENAMP
#include "ft_openamp.ld"
#endif
}
