/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *  
 * 
 * FilePath: vector.S
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:28:06
 * Description:  This file is for the initial vector table for the processor
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe     2021/7/3     first release
 */


#include "sdkconfig.h"
#include "fcpumode.h"

.globl _boot
.globl _vector_table
.globl FiqInterruptHandler
.globl IrqInterruptHandler
.globl SwInterruptHandler
.globl DataAbortInterruptHandler
.globl PrefetchAbortInterruptHandler
.globl UndefineInterruptHandler


.set EXCEPTION_FRAME_SIZE, (17*4)
.set EXCEPTION_PC_OFFSET, (15*4)
.set EXCEPTION_CPSR_OFFSET, (16*4)
.set EXCEPTION_SP_OFFSET, (13*4)
.set EXCEPTION_PC_OFFSET, (14*4)

/* allocatable and executable */
.section .vectors, "a"

.align 11
_vector_table:
    B	_boot
	B	Undefined
	B	SVCHandler
	B	PrefetchAbortHandler
	B	DataAbortHandler
	NOP	/* Placeholder for address exception vector*/
	B	IrqHandler
	B	FiqHandler

IrqHandler: /* 0x18 IRQ */
    stmdb	sp!,{r0-r3,r12,lr}		/* state save from compiled code*/


    vpush {d0-d7}
#if defined(CONFIG_CRYPTO_NEON_FP_ARMV8) || defined(CONFIG_VFPV4) || defined(CONFIG_VFPV3)
	vpush {d16-d31}
#endif
	vmrs r1, FPSCR
	push {r1}
	vmrs r1, FPEXC
	push {r1}

    /* Read value from the interrupt acknowledge register, which is stored in r0
	for future parameter and interrupt clearing use. */
    mrc p15, 0, r0, c12, c12, 0
    isb

    /* save r0 which is interrupt num, lr pushed to maintain alignment */
    PUSH	{r0, lr}

    bl FExceptionInterruptHandler	/* IRQ vector */   

    /* get r0 value which is interrupt num */
    POP	    {r0, lr}

    /* Write the value read from ICCIAR to ICCEOIR, end interrupt.*/
	mcr p15, 0, r0, c12, c12, 1
	isb

	pop 	{r1}
	vmsr    FPEXC, r1
	pop 	{r1}
	vmsr    FPSCR, r1
#if defined(CONFIG_CRYPTO_NEON_FP_ARMV8) || defined(CONFIG_VFPV4) || defined(CONFIG_VFPV3)
	vpop    {d16-d31}
#endif
	vpop    {d0-d7}


	ldmia	sp!,{r0-r3,r12,lr}		/* state restore from compiled code */

	subs	pc, lr, #4			/* adjust return */




.macro PushExceptionReg
    sub     sp, sp, #EXCEPTION_FRAME_SIZE         /* Sizeof(ExceptionFrame)  */
    stmia   sp, {r0 - r12}          /* Calling r0-r12                  */
    mov     r0, sp
    mrs     r6, spsr                /* Save CPSR                       */
    str     lr, [r0, #15*4]         /* Push PC                         */
    str     r6, [r0, #16*4]         /* Push CPSR                       */
    cps     #CURRENT_PE_MODE                   /* C运行环境所运行的模式           */
    str     sp, [r0, #13*4]         /* Save calling SP                 */
    str     lr, [r0, #14*4]         /* Save calling PC                 */
.endm


Undefined:  /* Undefined handler */
    PushExceptionReg
    cps #0x1B
    bl UndefineInterruptHandler
    bl .

SVCHandler:
    PushExceptionReg
    bl SwInterruptHandler
    bl .

PrefetchAbortHandler:
    PushExceptionReg
    bl PrefetchAbortInterruptHandler
    bl .

DataAbortHandler:
    PushExceptionReg
    bl DataAbortInterruptHandler
    bl .

FiqHandler:					/* FIQ vector handler */
    PushExceptionReg
    bl FiqInterruptHandler
    bl .

.end