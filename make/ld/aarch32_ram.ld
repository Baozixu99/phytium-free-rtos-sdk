#include "sdkconfig.h"

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_boot)


MEMORY
{
    ROM (rx) : ORIGIN = CONFIG_ROM_START_UP_ADDR, LENGTH = CONFIG_ROM_SIZE_MB * 1M
    RAM (rwx): ORIGIN = CONFIG_RAM_START_UP_ADDR, LENGTH = CONFIG_RAM_SIZE_MB * 1M
}

SECTIONS
{
    .vectors : {
        *(.boot)
        . = ALIGN(64);
        KEEP(*(.vectors))

    } > ROM

    .text : {
        *(.text*)
        *(.gnu.linkonce.t.*)
        *(.plt)
        *(.gnu_warning)
        *(.gcc_execpt_table)
        *(.glue_7)               /* glue arm to thumb code */
        *(.glue_7t)              /* glue thumb to arm code */        
        *(.vfp11_veneer)
        *(.ARM.extab)
        *(.gnu.linkonce.armextab.*)
        *(.note.gnu.build-id)
     } > ROM /* .text should linked to ROM */
    . = ALIGN(8);

    /* .ARM.exidx is sorted, so has to go in its own output section.  */
    .ARM :{
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        /* This is used by the startup in order to initialize the .data secion */
    } > ROM

    .interp : {
        KEEP (*(.interp))
    } > ROM

    .note-ABI-tag : {
        KEEP (*(.note-ABI-tag))
    } > ROM

    .rodata :{
        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)
        __rodata_end = .;
    } > ROM

    .rodata1 : {
        __rodata1_start = .;
        *(.rodata1)
        *(.rodata1.*)
        __rodata1_end = .;
    } > ROM

    .le_shell  (ALIGN(64)):{
        . = ALIGN(4096);
        _shell_command_start = .;
        KEEP (*(shellCommand))
        _shell_command_end = .;
        . = ALIGN(4096); /* align to page size */
        _shell_code_end = .;
    }> ROM

    .init :{
        KEEP (*(SORT_NONE(.init)))
    }> ROM

    .fini :{
        KEEP (*(SORT_NONE(.fini)))
    }> ROM

    .eh_frame :{
        KEEP (*(.eh_frame))
    }> ROM

    .ARM.extab :{
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    }> ROM

    .ARM.exidx :{
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    }> ROM

    .fini_array :{
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array ))
        PROVIDE_HIDDEN (__fini_array_end = .);
    }> ROM

    .dtors :{
        KEEP (*crtbegin.o(.dtors))
        KEEP (*crtbegin?.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
        . = ALIGN(16);
        __rom_end__ = .;
    }> ROM
    
    /* data segment, contains data that can be modified by the program at run-time,
       global and static variables that have a non-zero initial value will normally go here */
    .data : AT(__rom_end__){
        __data_start__ = .; /* .data section will be redise at runtime */
        *(.data)
        . = ALIGN(4096); /* align to page size */
        __data_end__ = .;        
    } > RAM /* link it all to RAM */

    /* unitialized data segment, variables explicitly initialized with zero to BSS */
    .bss (NOLOAD): {
        __bss_start__ = .;
        _bss_start = __bss_start__;        
        *(.bss)
        __bss_end__ = .;
        _bss_end = __bss_end__;                
    } > RAM

    .sbss (NOLOAD) : {
    __sbss_start__ = .;
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    __sbss_end__ = .;
    } > RAM

    .heap (NOLOAD):{
        . = ALIGN(16);
        __end__ = .;
        PROVIDE(end = .);
        HeapBase = .;
        _heap_start = .;
        . += CONFIG_HEAP_SIZE* 1M;
        _heap_end = .;
        HeapLimit = .; /* Add for _sbrk */
    } > RAM

    .stack (NOLOAD):{

        . = ALIGN(16); 
        _supervisor_stack_end = .;
        . += CONFIG_SVC_STACK_SIZE;
        _supervisor_stack = .;
        __supervisor_stack = _supervisor_stack;

        . = ALIGN(16);
        _irq_stack_end = .;
        . += CONFIG_IRQ_STACK_SIZE;
        __irq_stack = .;

        __sys_stack = .;
        . += CONFIG_SYS_STACK_SIZE;
        . = ALIGN(16);
        __sys_stack = .;

        _abort_stack_end = .;
        . += CONFIG_ABORT_STACK_SIZE;
        . = ALIGN(16);
        __abort_stack = .;

        _fiq_stack_end = .;
        . += CONFIG_FIQ_STACK_SIZE;
        . = ALIGN(16);
        __fiq_stack = .;

        _undef_stack_end = .;
        . += CONFIG_UNDEF_STACK_SIZE;
        . = ALIGN(16);
        __undef_stack = .;

    } > RAM

}
