#include "sdkconfig.h"

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(Startup_Aarch32)


HEAP_SIZE   = DEFINED(__heap_size__)           ? __heap_size__          : CONFIG_HEAP_SIZE;

/* pretend that 0x80100000 is ROM, 0x81000000 is RAM */
/* because vexpress-qemu is not able to emulate flash memory as ROM */
/* space is significant to avoid syntax error when linking */
MEMORY
{
    ROM (rx) : ORIGIN = CONFIG_ROM_START_UP_ADDR, LENGTH = CONFIG_ROM_SIZE_MB * 1M
    RAM (rwx): ORIGIN = CONFIG_RAM_START_UP_ADDR, LENGTH = CONFIG_RAM_SIZE_MB * 1M
}

SECTIONS
{
    .text : {
        _text_start = .;
        KEEP(*(.Startup_Aarch32))
        *(.vector_table)
        *(.text*)
        *(.rodata*) /* make life easier without rodata section */
        *(.glue_7)               /* glue arm to thumb code */
        *(.glue_7t)              /* glue thumb to arm code */        
     } > ROM /* .text should linked to ROM */
    . = ALIGN(8);

    /* .ARM.exidx is sorted, so has to go in its own output section.  */
    .ARM :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        /* This is used by the startup in order to initialize the .data secion */
    } > ROM
    
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
    
    
    . = ALIGN(4096); /* align to page size */
    _text_end = .;
    
    /* data segment, contains data that can be modified by the program at run-time,
       global and static variables that have a non-zero initial value will normally go here */
    .data : AT(_text_end){
        _data_start = .; /* .data section will be redise at runtime */
        *(.data)
        . = ALIGN(4096); /* align to page size */
        _data_end = .;        
    } > RAM /* link it all to RAM */

    /* unitialized data segment, variables explicitly initialized with zero to BSS */
    .bss : {
        __bss_start__ = .;
        _bss_start = __bss_start__;        
        *(.bss)
        __bss_end__ = .;
        _bss_end = __bss_end__;                
    } > RAM

    .heap :
    {
        . = ALIGN(8);
        __end__ = .;
        PROVIDE(end = .);
        HeapBase = .;
        _heap_start = .;
        . += HEAP_SIZE;
        _heap_end = .;
        HeapLimit = .; /* Add for _sbrk */
    } > RAM

    _fiq_stack_start = ADDR(.heap) + SIZEOF(.heap);
    _fiq_stack_end = _fiq_stack_start + CONFIG_STACK_SIZE; /* 64 KB */

    _irq_stack_start = _fiq_stack_end;
    _irq_stack_end = _irq_stack_start + CONFIG_STACK_SIZE; /* 64 KB */

    _sys_stack_start = _irq_stack_end;
    _sys_stack_end  = _sys_stack_start + CONFIG_STACK_SIZE; /* 64 KB */

    /* put svc at end and start up with SVC mode */
    _svc_stack_start = _sys_stack_end;
    _svc_stack_end = _svc_stack_start + CONFIG_STACK_SIZE; /* 64 KB */ 

    _irq_stack_size = _irq_stack_end - _irq_stack_start;
    _fiq_stack_size = _fiq_stack_end - _fiq_stack_start;
    _sys_stack_size = _sys_stack_end - _sys_stack_start;
    _svc_stack_size = _svc_stack_end - _svc_stack_start;
}
