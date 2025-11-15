/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fboot.h
 * Created Date: 2025-03-19 10:32:06
 * Last Modified: 2025-03-19 10:33:08
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */
#ifndef __FBOOT_H__
#define __FBOOT_H__

#ifdef __ASSEMBLER__

#define FPSR_F_BIT         0x00000040
#define FPSR_I_BIT         0x00000080
#define FPSR_A_BIT         0x00000100
#define FPSR_D_BIT         0x00000200

#define FPSR_MODE_EL0t     0x00000000
#define FPSR_MODE_EL1t     0x00000004
#define FPSR_MODE_EL1h     0x00000005
#define FPSR_MODE_EL2t     0x00000008
#define FPSR_MODE_EL2h     0x00000009
#define FPSR_MODE_SVC_32   0x00000013


.macro disable_mmu sctlr tmp
    mrs     \tmp, \sctlr
    bic     \tmp, \tmp, #(1 << 0)
    bic     \tmp, \tmp, #(1 << 2)
    bic     \tmp, \tmp, #(1 << 12)
    msr     \sctlr, \tmp
    isb
.endm

.macro dcache op
    dsb     sy
    mrs     x0, clidr_el1
    and     x3, x0, #0x7000000
    lsr     x3, x3, #23

    cbz     x3, finished_\op
    mov     x10, #0

loop1_\op:
    add     x2, x10, x10, lsr #1
    lsr     x1, x0, x2
    and     x1, x1, #7
    cmp     x1, #2
    b.lt    skip_\op

    msr     csselr_el1, x10
    isb

    mrs     x1, ccsidr_el1
    and     x2, x1, #7
    add     x2, x2, #4
    mov     x4, #0x3ff
    and     x4, x4, x1, lsr #3
    clz     w5, w4
    mov     x7, #0x7fff
    and     x7, x7, x1, lsr #13

loop2_\op:
    mov     x9, x4

loop3_\op:
    lsl     x6, x9, x5
    orr     x11, x10, x6
    lsl     x6, x7, x2
    orr     x11, x11, x6
    dc      \op, x11
    subs    x9, x9, #1
    b.ge    loop3_\op
    subs    x7, x7, #1
    b.ge    loop2_\op

skip_\op:
    add     x10, x10, #2
    cmp     x3, x10
    b.gt    loop1_\op

finished_\op:
    mov     x10, #0
    msr     csselr_el1, x10
    dsb     sy
    isb
.endm

#else /* !__ASSEMBLER__ */
#warning "Including assembly-specific header in C code"
#endif
#endif // !