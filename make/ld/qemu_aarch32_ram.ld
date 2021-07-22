OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(system_vectors)

HEAP_SIZE   = DEFINED(__heap_size__)           ? __heap_size__          : 0x0400;

MEMORY
{
    ROM (rx) : ORIGIN = 0x60000000, LENGTH = 0x100000
    RAM (rwx): ORIGIN = 0x70000000, LENGTH = 0x3200000
}

SECTIONS
{
    .text : {
        _text_start = .;
        /*KEEP(*(.Startup_Aarch32))*/
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
        _bss_start = .;
        *(.bss)
        . = ALIGN(4096); /* align to page size */
        _bss_end = .;        
    } > RAM

    .heap :
    {
        . = ALIGN(8);
        __end__ = .;
        PROVIDE(end = .);
        __HeapBase = .;
        . += HEAP_SIZE;
        __HeapLimit = .;
        __heap_limit = .; /* Add for _sbrk */
    } > RAM

    _fiq_stack_start = ADDR(.heap) + SIZEOF(.heap);
    _fiq_stack_end = _fiq_stack_start + 0x1000; /* 64 KB */

    _irq_stack_start = _fiq_stack_end;
    _irq_stack_end = _irq_stack_start + 0x1000; /* 64 KB */

    _sys_stack_start = _irq_stack_end;
    _sys_stack_end  = _sys_stack_start + 0x1000; /* 64 KB */

    /* put svc at end and start up with SVC mode */
    _svc_stack_start = _sys_stack_end;
    _svc_stack_end = _svc_stack_start + 0x1000; /* 64 KB */

    _irq_stack_size = _irq_stack_end - _irq_stack_start;
    _fiq_stack_size = _fiq_stack_end - _fiq_stack_start;
    _sys_stack_size = _sys_stack_end - _sys_stack_start;
    _svc_stack_size = _svc_stack_end - _svc_stack_start;
}
