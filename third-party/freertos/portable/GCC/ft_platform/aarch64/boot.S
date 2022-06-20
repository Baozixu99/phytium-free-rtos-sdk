.global _boot

.global _vector_table


.set vector_base, _vector_table
.set el1_stack, __el1_stack



.section .boot,"ax"

_boot:
        bl  FTraceUartInit
        bl  FTraceBootup
        mrs	x0, CurrentEL
        cmp x0, 0xc
        b.eq    El3Entry
        cmp	x0, 0x8
        b.eq	El2Entry
        cmp	x0, 0x4
        b.eq	El1Entry
        bne	hang
    El3Entry:
        /*  initialize sctlr_el2 and hcr_el2 to save values before entering el2.*/
        msr sctlr_el2, xzr
        msr hcr_el2, xzr
        /*  determine the el2 execution state.*/
        mrs x0, scr_el3
        orr x0, x0, #(1<<10) 
        /*  rw el2 execution state is aarch64.*/
        orr x0, x0, #(1<<0) 
        /*  ns el1 is non-secure world.*/
        msr scr_el3, x0
        mov x0, #0b01001 
        /*  daif=0000 */
        msr spsr_el3, x0 
        /*  m[4:0]=01001 el2h must match scr_el3.rw*/

        bl  FTraceEL3
        /*  determine el2 entry.*/
        adr x0, El2Entry 
        /*  El2Entry points to the first instruction of */
        msr elr_el3, x0 
        /*  el2 code.*/
        eret
    El2Entry:
        mov x1, #0x0
        msr sctlr_el1, x1
        /*  initialize the sctlr_el1 register before entering el1.*/
        msr sctlr_el1, xzr
        mrs x0, hcr_el2
        orr x0, x0, #(1<<31) 
        bic x0, x0, #(1<<27) /* HCR_EL2.TGE = 0 */
        /*  rw=1 el1 execution state is aarch64.*/
        msr hcr_el2, x0
        mov x0, #0b00101 
        /*  daif=0000 */
        msr spsr_el2, x0 
        /*  m[4:0]=00101 el1h must match hcr_el2.rw.*/

        bl  FTraceEL2
        adr x0, El1Entry 
        /*  El1Entry points to the first instruction of */
        msr elr_el2, x0 
        /*  el1 code.*/
        eret

El1Entry:
InitEl1:
    mov      x0, #0
	mov      x1, #0
	mov      x2, #0
	mov      x3, #0
	mov      x4, #0
	mov      x5, #0
	mov      x6, #0
	mov      x7, #0
	mov      x8, #0
	mov      x9, #0
	mov      x10, #0
	mov      x11, #0
	mov      x12, #0
	mov      x13, #0
	mov      x14, #0
	mov      x15, #0
	mov      x16, #0
	mov      x17, #0
	mov      x18, #0
	mov      x19, #0
	mov      x20, #0
	mov      x21, #0
	mov      x22, #0
	mov      x23, #0
	mov      x24, #0
	mov      x25, #0
	mov      x26, #0
	mov      x27, #0
	mov      x28, #0
	mov      x29, #0
	mov      x30, #0
    mov	    x1,#0x0
    msr     SCTLR_EL1, x1
    isb

    /* Set vector table base address */
    ldr x1,=vector_base
    msr VBAR_EL1,x1


    /* Trap floating point access only in case of standalone BSP */
    mrs	x0, CPACR_EL1
    orr     x0, x0, #(0x3 << 20)    /*This control does not cause any instructions to be trapped. */
	msr	CPACR_EL1, x0
    isb


    /* Define stack pointer for current exception level */
    ldr x2,=el1_stack
    mov	 sp,x2

    bl   FTraceEL1
    bl 	 _startup		/* jump to start */


hang:
	wfi
    b   HangPrint
	b	hang
