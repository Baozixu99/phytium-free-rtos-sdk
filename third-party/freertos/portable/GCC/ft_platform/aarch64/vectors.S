
.globl _boot
.globl _vector_table
.globl _freertos_vector_table

.globl FiqInterrupt
.globl IrqInterrupt
.globl SerrorInterrupt
.globl SynchronousInterrupt
.globl fpu_status




.macro SaveRegister
	stp	X0,X1, [sp,#-0x10]!
	stp	X2,X3, [sp,#-0x10]!
	stp	X4,X5, [sp,#-0x10]!
	stp	X6,X7, [sp,#-0x10]!
	stp	X8,X9, [sp,#-0x10]!
	stp	X10,X11, [sp,#-0x10]!
	stp	X12,X13, [sp,#-0x10]!
	stp	X14,X15, [sp,#-0x10]!
	stp	X16,X17, [sp,#-0x10]!
	stp X18,X19, [sp,#-0x10]!
	stp X29,X30, [sp,#-0x10]!
.endm


.macro RestoreRegister
	ldp X29,X30, [sp], #0x10
	ldp X18,X19, [sp], #0x10
	ldp	X16,X17, [sp], #0x10
	ldp	X14,X15, [sp], #0x10
	ldp	X12,X13, [sp], #0x10
	ldp	X10,X11, [sp], #0x10
	ldp	X8,X9, [sp], #0x10
	ldp	X6,X7, [sp], #0x10
	ldp	X4,X5, [sp], #0x10
	ldp	X2,X3, [sp], #0x10
	ldp	X0,X1, [sp], #0x10
.endm


.macro SaveFloatregister
	bl FloatSave
/* Load the floating point context array address from fpu_context_base */
	ldr	x1,=fpu_context_base
	ldr	x0, [x1]

/* Save all the floating point register to the array */
	stp	q0,q1, [x0], #0x20
	stp	q2,q3, [x0], #0x20
	stp	q4,q5, [x0], #0x20
	stp	q6,q7, [x0], #0x20
	stp	q8,q9, [x0], #0x20
	stp	q10,q11, [x0], #0x20
	stp	q12,q13, [x0], #0x20
	stp	q14,q15, [x0], #0x20
	stp	q16,q17, [x0], #0x20
	stp	q18,q19, [x0], #0x20
	stp	q20,q21, [x0], #0x20
	stp	q22,q23, [x0], #0x20
	stp	q24,q25, [x0], #0x20
	stp	q26,q27, [x0], #0x20
	stp	q28,q29, [x0], #0x20
	stp	q30,q31, [x0], #0x20
	mrs	x2, FPCR
	mrs	x3, FPSR
	stp	x2, x3, [x0], #0x10

/* Save current address of floating point context array to fpu_context_base */
	str	x0, [x1]
.endm

.macro RestoreFloatRegister

/* Restore the address of floating point context array from fpu_context_base */
	ldr	x1,=fpu_context_base
	ldr	x0, [x1]

/* Restore all the floating point register from the array */
	ldp	x2, x3, [x0,#-0x10]!
	msr	FPCR, x2
	msr	FPSR, x3
	ldp	q30,q31, [x0,#-0x20]!
	ldp	q28,q29, [x0,#-0x20]!
	ldp	q26,q27, [x0,#-0x20]!
	ldp	q24,q25, [x0,#-0x20]!
	ldp	q22,q23, [x0,#-0x20]!
	ldp	q20,q21, [x0,#-0x20]!
	ldp	q18,q19, [x0,#-0x20]!
	ldp	q16,q17, [x0,#-0x20]!
	ldp	q14,q15, [x0,#-0x20]!
	ldp	q12,q13, [x0,#-0x20]!
	ldp	q10,q11, [x0,#-0x20]!
	ldp	q8,q9, [x0,#-0x20]!
	ldp	q6,q7, [x0,#-0x20]!
	ldp	q4,q5, [x0,#-0x20]!
	ldp	q2,q3, [x0,#-0x20]!
	ldp	q0,q1, [x0,#-0x20]!

/* Save current address of floating point context array to fpu_context_base */
	str	x0, [x1]
.endm

.org 0

.section .vectors, "a"

_vector_table:
.set	VBAR, _vector_table
.org 	VBAR

	b	_boot
.org (VBAR + 0x200)
	b	SynchronousInterruptHandler

.org (VBAR + 0x280)
	b	IRQInterruptHandler

.org (VBAR + 0x300)
	b	FIQInterruptHandler

.org (VBAR + 0x380)
	b	SErrorInterruptHandler


SynchronousInterruptHandler:
	SaveRegister
/* Check if the Synchronous abort is occurred due to floating point access. */
	mrs	x0, ESR_EL1

	and	x0, x0, #(0x3F << 26)
	mov	x1, #(0x7 << 26)
	cmp	x0, x1
/* If exception is not due to floating point access go to synchronous handler */
	bne	Synchronoushandler

/*
 * If excpetion occurred due to floating point access, Enable the floating point
 * access i.e. do not trap floating point instruction
 */

	mrs	x1,CPACR_EL1
	orr	x1, x1, #(0x1<<20)
	msr	CPACR_EL1, x1
	isb

/* If the floating point access was previously enabled, store FPU context
 * registers(StoreFloat).
 */
	ldr	x0, =fpu_status
	ldrb	w1,[x0]
	cbnz	w1, StoreFloat
/*
 * If the floating point access was not enabled previously, save the status of
 * floating point accessibility i.e. enabled and store floating point context
 * array address(fpu_context) to fpu_context_base.
 */
	mov	w1, #0x1
	strb	w1, [x0]
	ldr	x0, =fpu_context
	ldr	x1, =fpu_context_base
	str	x0,[x1]
	b	RestoreContext
StoreFloat:
	SaveFloatRegister
	b	RestoreContext
Synchronoushandler: /* 此处还需要进行压栈处理，位函数内提供打印信息 */
	mrs 	x0, CPACR_EL1
	mrs 	x1, ELR_EL1
	mrs		x2, SPSR_EL1
	stp	    x0, x1, [sp,#-0x10]!
	mov		x21, sp 	
	add		x21, x21, #208
	stp		x21,x21, [sp,#-0x10]!
	mov 	x0, sp

	bl		SynchronousInterrupt
RestoreContext:
	RestoreRegister
	eret


IRQInterruptHandler:
	SaveRegister
/* Save the status of SPSR, ELR and CPTR to stack */
	mrs 	x0, CPACR_EL1
	mrs 	x1, ELR_EL1
	mrs		x2, SPSR_EL1

	stp	x0, x1, [sp,#-0x10]!
	str	x2, [sp,#-0x10]!

/* Trap floating point access */
	mrs	x1,CPACR_EL1
	bic	x1, x1, #(0x1<<20)
	msr	CPACR_EL1, x1
	isb

	bl	IRQInterrupt
/*
 * If floating point access is enabled during interrupt handling,
 * restore floating point registers.
 */
	mrs	x0,CPACR_EL1
	ands	x0, x0, #(0x1<<20)
	beq	RestorePrevState

	RestoreFloatRegister

/* Restore the status of SPSR, ELR and CPTR from stack */
RestorePrevState:
	ldr	x2,[sp],0x10
	ldp	x0, x1, [sp],0x10

	msr	CPACR_EL1, x0
	msr	ELR_EL1, x1
	msr	SPSR_EL1, x2

	RestoreRegister
	eret

FIQInterruptHandler:

	SaveRegister

	bl	FIQInterrupt

	RestoreRegister

	eret

SErrorInterruptHandler:

	SaveRegister
	/* Save the status of SPSR, ELR and CPTR to stack */
	mrs 	x0, CPACR_EL1
	mrs 	x1, ELR_EL1
	mrs		x2, SPSR_EL1
	stp	    x0, x1, [sp,#-0x10]!
	mov		x21, sp 	
	add		x21, x21, #208
	stp		x21,x21, [sp,#-0x10]!
	mov 	x0, sp

	mov 	x0, sp
	bl      SErrorInterrupt

	RestoreRegister

	eret




.set	FREERTOS_VBAR, (VBAR+0x2000)

.org(FREERTOS_VBAR)
_freertos_vector_table:
	b	FreeRTOS_SWI_Handler

.org (FREERTOS_VBAR + 0x80)
	b	FreeRTOS_IRQ_Handler

.org (FREERTOS_VBAR + 0x100)
	b	.

.org (FREERTOS_VBAR + 0x180)
	b	.

.org (FREERTOS_VBAR + 0x200)
	b	FreeRTOS_SWI_Handler

.org (FREERTOS_VBAR + 0x280)
	b	FreeRTOS_IRQ_Handler

.org (FREERTOS_VBAR + 0x300)
	b	.

.org (FREERTOS_VBAR + 0x380)
	b	SErrorInterruptHandler		/* SError/vSError */

.org (FREERTOS_VBAR + 0x400)
	b	.

.org (FREERTOS_VBAR + 0x480)
	b	.

.org (FREERTOS_VBAR + 0x500)
	b	.

.org (FREERTOS_VBAR + 0x580)
	b	.

.org (FREERTOS_VBAR + 0x600)
	b	.

.org (FREERTOS_VBAR + 0x680)
	b	.

.org (FREERTOS_VBAR + 0x700)
	b	.

.org (FREERTOS_VBAR + 0x780)
	b	.

.org (FREERTOS_VBAR + 0x800)


.align 8

.end