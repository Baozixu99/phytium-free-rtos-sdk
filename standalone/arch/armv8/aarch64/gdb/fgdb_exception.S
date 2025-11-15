/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *	 https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *  
 * Modified from FreeBSD sys/arm64/arm64/exception.S with LICENSE 
 *
 * Copyright (c) 2014 Andrew Turner
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * FilePath: fgdb_exception.S
 * Date: 2024-01-15 14:53:41
 * LastEditTime: 2024-01-16 17:31:50
 * Description:  This file is for the initial vector table for the processor
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2025/8/19	 first release
 */

.org 0

#define	 EXCP_BRKPT_EL1		0x31	/* Hardware breakpoint, from same EL */
#define	 EXCP_SOFTSTP_EL1	0x33	/* Software Step, from same EL */
#define	 EXCP_WATCHPT_EL1	0x35	/* Watchpoint, from same EL */
#define	 EXCP_BRK			0x3c	/* Breakpoint caused by 'brk' */

#define	DAIF_D			(1 << 3)
#define	DAIF_A			(1 << 2)
#define	DAIF_I			(1 << 1)
#define	DAIF_F			(1 << 0)
#define	DAIF_ALL		(DAIF_D | DAIF_A | DAIF_I | DAIF_F)

#define TF_ELR  0x10
#define TF_ESR  0x20
#define TF_LR   0x8
#define TF_SIZE 0x120
#define TF_SPSR 0x18
#define TF_SP   0
#define TF_X	0x30

.globl GdbDebugInterruptHandler

.section .vectors, "a"

/*
 * This is limited to 28 instructions as it's placed in the exception vector
 * slot that is 32 instructions long. We need one for the branch, and three
 * for the prologue.
 */
.macro	save_registers_head
	mov	x18, sp
	stp	x0,  x1,  [sp, #(TF_X - TF_SIZE - 128)]!
	stp	x2,  x3,  [sp, #(2  * 8)]
	stp	x4,  x5,  [sp, #(4  * 8)]
	stp	x6,  x7,  [sp, #(6  * 8)]
	stp	x8,  x9,  [sp, #(8  * 8)]
	stp	x10, x11, [sp, #(10 * 8)]
	stp	x12, x13, [sp, #(12 * 8)]
	stp	x14, x15, [sp, #(14 * 8)]
	stp	x16, x17, [sp, #(16 * 8)]
	stp	x18, x19, [sp, #(18 * 8)]
	stp	x20, x21, [sp, #(20 * 8)]
	stp	x22, x23, [sp, #(22 * 8)]
	stp	x24, x25, [sp, #(24 * 8)]
	stp	x26, x27, [sp, #(26 * 8)]
	stp	x28, x29, [sp, #(28 * 8)]
	mrs	x10, elr_el1
	mrs	x11, spsr_el1
	mrs	x12, esr_el1
	mrs	x13, far_el1
	stp	x18,  lr, [sp, #(TF_SP - TF_X)]!
	stp	x10, x11, [sp, #(TF_ELR)]
	stp	x12, x13, [sp, #(TF_ESR)]
	/*
	 * Unmask debug and SError exceptions.
	 * For EL1, debug exceptions are conditionally unmasked in
	 * do_el1h_sync().
	 */
	msr	daifclr, #(DAIF_A)
.endm

.macro	restore_registers
	/*
	 * Mask all exceptions, x18 may change in the interrupt exception
	 * handler.
	 */
	msr	daifset, #(DAIF_ALL)
	ldp	x18,  lr, [sp, #(TF_SP)]
	ldp	x10, x11, [sp, #(TF_ELR)]
	msr	spsr_el1, x11
	msr	elr_el1, x10
	ldp	x0,  x1,  [sp, #(TF_X + 0  * 8)]
	ldp	x2,  x3,  [sp, #(TF_X + 2  * 8)]
	ldp	x4,  x5,  [sp, #(TF_X + 4  * 8)]
	ldp	x6,  x7,  [sp, #(TF_X + 6  * 8)]
	ldp	x8,  x9,  [sp, #(TF_X + 8  * 8)]
	ldp	x10, x11, [sp, #(TF_X + 10 * 8)]
	ldp	x12, x13, [sp, #(TF_X + 12 * 8)]
	ldp	x14, x15, [sp, #(TF_X + 14 * 8)]
	ldp	x16, x17, [sp, #(TF_X + 16 * 8)]
	ldr		 x29, [sp, #(TF_X + 29 * 8)]
	mov	sp, x18
.endm

GdbDebugInterruptHandler:
	save_registers_head

	mrs 	x0, ESR_EL1
	lsr 	x0, x0, #26
	and 	x0, x0, #0x3F

	cmp 	x0, EXCP_BRKPT_EL1 /* EC_BREAKPT */
	b.eq 	1f
	cmp 	x0, EXCP_SOFTSTP_EL1 /* EC_SOFTSTP */
	b.eq 	1f
	cmp 	x0, EXCP_WATCHPT_EL1 /* EC_WATCHPT */
	b.eq 	1f
	cmp 	x0, EXCP_BRK /* EC_BRK64 */
	b.eq 	1f

	b	 	2f
1:
	mov 	x0, sp
	bl		FGdbDebugInterrupt

	/* Use barrier instructions to ensure ordering of memory operations. */
	dsb	 	sy
	isb

	restore_registers
	eret
2:
	restore_registers
	mov 	x0, sp
	b	 	SynchronousInterruptHandler