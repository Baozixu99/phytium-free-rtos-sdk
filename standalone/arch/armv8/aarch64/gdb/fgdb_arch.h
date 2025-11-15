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
 * Modified from FreeBSD sys/arm64/include/debug_monitor.h with LICENSE
 * 
 * Copyright (c) 2014 The FreeBSD Foundation
 *
 * This software was developed by Semihalf under
 * the sponsorship of the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * FilePath: fgdb_arch.h
 * Created Date: 2024-08-27 17:05:14
 * Last Modified: 2024-08-27 17:34:17
 * Description:  This file is for gdb stub aarch64 function definition
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0   zhugengyu   2025/8/12    init commit
 */

#ifndef FGDB_ARCH_H
#define FGDB_ARCH_H

/***************************** Include Files *********************************/
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
/*
 * brk instruction encoding
 */
#define BREAK_INSTR_SIZE               4U
#define ESR_ELx_BRK64_ISS_COMMENT_MASK 0xFFFF

/*
 * #imm16 values used for BRK instruction generation
 * 0x401: for compile time BRK instruction
 */
#define GDB_COMPILED_DBG_BRK_IMM       0x401

#define FGDB_INSERT_BRK()       \
    __asm__ volatile("dsb st\n" \
                     "brk %0\n" \
                     "dsb st\n" \
                     :          \
                     : "I"(GDB_COMPILED_DBG_BRK_IMM));

#define READ_SPECIALREG(reg)                                        \
    ({                                                              \
        uint64_t _val;                                              \
        __asm __volatile("mrs	%0, " __STRING(reg) : "=&r"(_val)); \
        _val;                                                       \
    })
#define WRITE_SPECIALREG(reg, _val) \
    __asm __volatile("msr	" __STRING(reg) ", %0" : : "r"((uint64_t)_val))

#define ID_AA64DFR0_BRPs_SHIFT        12
#define ID_AA64DFR0_BRPs_MASK         ((0xfUL) << ID_AA64DFR0_BRPs_SHIFT)
#define ID_AA64DFR0_BRPs_VAL(x)       ((((x) >> ID_AA64DFR0_BRPs_SHIFT) & 0xf) + 1)
#define ID_AA64DFR0_WRPs_SHIFT        20
#define ID_AA64DFR0_WRPs_MASK         ((0xfUL) << ID_AA64DFR0_WRPs_SHIFT)
#define ID_AA64DFR0_WRPs_VAL(x)       ((((x) >> ID_AA64DFR0_WRPs_SHIFT) & 0xf) + 1)

#define PSR_AARCH32                   0x00000010UL
#define PSR_F                         0x00000040UL
#define PSR_I                         0x00000080UL
#define PSR_A                         0x00000100UL
#define PSR_D                         0x00000200UL
#define PSR_DAIF                      (PSR_D | PSR_A | PSR_I | PSR_F)
/* The default DAIF mask. These bits are valid in spsr_el1 and daif */
#define PSR_DAIF_DEFAULT              (PSR_F)
#define PSR_BTYPE                     0x00000c00UL
#define PSR_SSBS                      0x00001000UL
#define PSR_ALLINT                    0x00002000UL
#define PSR_IL                        0x00100000UL
#define PSR_SS                        0x00200000UL
#define PSR_PAN                       0x00400000UL
#define PSR_UAO                       0x00800000UL
#define PSR_DIT                       0x01000000UL
#define PSR_TCO                       0x02000000UL
#define PSR_V                         0x10000000UL
#define PSR_C                         0x20000000UL
#define PSR_Z                         0x40000000UL
#define PSR_N                         0x80000000UL
#define PSR_FLAGS                     0xf0000000UL
/* PSR fields that can be set from 32-bit and 64-bit processes */
#define PSR_SETTABLE_32               PSR_FLAGS
#define PSR_SETTABLE_64               (PSR_FLAGS | PSR_SS)

/* DAIFSet/DAIFClear */
#define DAIF_D                        (1 << 3)
#define DAIF_A                        (1 << 2)
#define DAIF_I                        (1 << 1)
#define DAIF_F                        (1 << 0)
#define DAIF_ALL                      (DAIF_D | DAIF_A | DAIF_I | DAIF_F)
#define DAIF_INTR                     (DAIF_I) /* All exceptions that pass */
                                               /* through the intr framework */

#define EXCP_BRKPT_EL0                0x30 /* Hardware breakpoint, from same EL */
#define EXCP_BRKPT_EL1                0x31 /* Hardware breakpoint, from same EL */
#define EXCP_SOFTSTP_EL0              0x32 /* Software Step, from lower EL */
#define EXCP_SOFTSTP_EL1              0x33 /* Software Step, from same EL */
#define EXCP_WATCHPT_EL0              0x34 /* Watchpoint, from lower EL */
#define EXCP_WATCHPT_EL1              0x35 /* Watchpoint, from same EL */
#define EXCP_BRKPT_32                 0x38 /* 32bits breakpoint */
#define EXCP_BRK                      0x3c /* Breakpoint */

#define SIGTRAP                       5 /* trace trap (not reset when caught) */
#define SIGEMT                        7 /* EMT instruction */

#define GDB_NREGS                     68
/* G.5.1.1 AArch64 core registers feature */
#define GDB_REG_X0                    0
#define GDB_REG_X19                   19
#define GDB_REG_X29                   29
#define GDB_REG_LR                    30
#define GDB_REG_SP                    31
#define GDB_REG_PC                    32
#define GDB_REG_CSPR                  33
/* G.5.1.2 AArch64 floating-point registers feature */
#define GDB_REG_V0                    34
#define GDB_REG_V31                   65
#define GDB_REG_FPSR                  66
#define GDB_REG_FPCR                  67

/* Watchpoints/breakpoints control register bitfields */
#define DBG_WATCH_CTRL_LEN_1          (0x1 << 5)
#define DBG_WATCH_CTRL_LEN_2          (0x3 << 5)
#define DBG_WATCH_CTRL_LEN_4          (0xf << 5)
#define DBG_WATCH_CTRL_LEN_8          (0xff << 5)
#define DBG_WATCH_CTRL_LEN_MASK(x)    ((x) & (0xff << 5))
#define DBG_WATCH_CTRL_EXEC           (0x0 << 3)
#define DBG_WATCH_CTRL_LOAD           (0x1 << 3)
#define DBG_WATCH_CTRL_STORE          (0x2 << 3)
#define DBG_WATCH_CTRL_ACCESS_MASK(x) ((x) & (0x3 << 3))

/* Common for breakpoint and watchpoint */
#define DBG_WB_CTRL_EL1               (0x1 << 1)
#define DBG_WB_CTRL_EL0               (0x2 << 1)
#define DBG_WB_CTRL_ELX_MASK(x)       ((x) & (0x3 << 1))
#define DBG_WB_CTRL_E                 (0x1 << 0)

#define DBG_REG_BASE_BVR              0
#define DBG_REG_BASE_BCR              (DBG_REG_BASE_BVR + 16)
#define DBG_REG_BASE_WVR              (DBG_REG_BASE_BCR + 16)
#define DBG_REG_BASE_WCR              (DBG_REG_BASE_WVR + 16)


/* Watchpoint/breakpoint helpers */
#define DBG_WB_WVR                    "wvr"
#define DBG_WB_WCR                    "wcr"
#define DBG_WB_BVR                    "bvr"
#define DBG_WB_BCR                    "bcr"

#define DBG_WB_READ(reg, num, val)                                   \
    do                                                               \
    {                                                                \
        __asm __volatile("mrs %0, dbg" reg #num "_el1" : "=r"(val)); \
    } while (0)

#define DBG_WB_WRITE(reg, num, val)                                 \
    do                                                              \
    {                                                               \
        __asm __volatile("msr dbg" reg #num "_el1, %0" ::"r"(val)); \
    } while (0)

#define READ_WB_REG_CASE(reg, num, offset, val) \
    case (num + offset):                        \
        DBG_WB_READ(reg, num, val);             \
        break

#define WRITE_WB_REG_CASE(reg, num, offset, val) \
    case (num + offset):                         \
        DBG_WB_WRITE(reg, num, val);             \
        break

#define SWITCH_CASES_READ_WB_REG(reg, offset, val) \
    READ_WB_REG_CASE(reg, 0, offset, val);         \
    READ_WB_REG_CASE(reg, 1, offset, val);         \
    READ_WB_REG_CASE(reg, 2, offset, val);         \
    READ_WB_REG_CASE(reg, 3, offset, val);         \
    READ_WB_REG_CASE(reg, 4, offset, val);         \
    READ_WB_REG_CASE(reg, 5, offset, val);         \
    READ_WB_REG_CASE(reg, 6, offset, val);         \
    READ_WB_REG_CASE(reg, 7, offset, val);         \
    READ_WB_REG_CASE(reg, 8, offset, val);         \
    READ_WB_REG_CASE(reg, 9, offset, val);         \
    READ_WB_REG_CASE(reg, 10, offset, val);        \
    READ_WB_REG_CASE(reg, 11, offset, val);        \
    READ_WB_REG_CASE(reg, 12, offset, val);        \
    READ_WB_REG_CASE(reg, 13, offset, val);        \
    READ_WB_REG_CASE(reg, 14, offset, val);        \
    READ_WB_REG_CASE(reg, 15, offset, val)

#define SWITCH_CASES_WRITE_WB_REG(reg, offset, val) \
    WRITE_WB_REG_CASE(reg, 0, offset, val);         \
    WRITE_WB_REG_CASE(reg, 1, offset, val);         \
    WRITE_WB_REG_CASE(reg, 2, offset, val);         \
    WRITE_WB_REG_CASE(reg, 3, offset, val);         \
    WRITE_WB_REG_CASE(reg, 4, offset, val);         \
    WRITE_WB_REG_CASE(reg, 5, offset, val);         \
    WRITE_WB_REG_CASE(reg, 6, offset, val);         \
    WRITE_WB_REG_CASE(reg, 7, offset, val);         \
    WRITE_WB_REG_CASE(reg, 8, offset, val);         \
    WRITE_WB_REG_CASE(reg, 9, offset, val);         \
    WRITE_WB_REG_CASE(reg, 10, offset, val);        \
    WRITE_WB_REG_CASE(reg, 11, offset, val);        \
    WRITE_WB_REG_CASE(reg, 12, offset, val);        \
    WRITE_WB_REG_CASE(reg, 13, offset, val);        \
    WRITE_WB_REG_CASE(reg, 14, offset, val);        \
    WRITE_WB_REG_CASE(reg, 15, offset, val)


#define MRS_MASK           0xfff00000
#define MRS_VALUE          0xd5300000
#define MRS_SPECIAL(insn)  ((insn)&0x000fffe0)
#define MRS_REGISTER(insn) ((insn)&0x0000001f)
#define MRS_Op0_SHIFT      19
#define MRS_Op0_MASK       0x00080000
#define MRS_Op1_SHIFT      16
#define MRS_Op1_MASK       0x00070000
#define MRS_CRn_SHIFT      12
#define MRS_CRn_MASK       0x0000f000
#define MRS_CRm_SHIFT      8
#define MRS_CRm_MASK       0x00000f00
#define MRS_Op2_SHIFT      5
#define MRS_Op2_MASK       0x000000e0
#define MRS_Rt_SHIFT       0
#define MRS_Rt_MASK        0x0000001f
#define __MRS_REG(op0, op1, crn, crm, op2)                                            \
    (((op0) << MRS_Op0_SHIFT) | ((op1) << MRS_Op1_SHIFT) | ((crn) << MRS_CRn_SHIFT) | \
     ((crm) << MRS_CRm_SHIFT) | ((op2) << MRS_Op2_SHIFT))
#define MRS_REG(reg)    __MRS_REG(reg##_op0, reg##_op1, reg##_CRn, reg##_CRm, reg##_op2)

/* MDSCR_EL1 - Monitor Debug System Control Register */
#define MDSCR_EL1       MRS_REG(MDSCR_EL1)
#define MDSCR_EL1_op0   2
#define MDSCR_EL1_op1   0
#define MDSCR_EL1_CRn   0
#define MDSCR_EL1_CRm   2
#define MDSCR_EL1_op2   2
#define MDSCR_SS_SHIFT  0
#define MDSCR_SS        ((0x1UL) << MDSCR_SS_SHIFT)
#define MDSCR_KDE_SHIFT 13
#define MDSCR_KDE       ((0x1UL) << MDSCR_KDE_SHIFT)
#define MDSCR_MDE_SHIFT 15
#define MDSCR_MDE       ((0x1UL) << MDSCR_MDE_SHIFT)
/************************** Function Prototypes ******************************/
typedef struct FGdb_ FGdb;
typedef struct FGdbTrapframe_
{
    uint64_t tf_sp;
    uint64_t tf_lr;
    uint64_t tf_elr;
    uint64_t tf_spsr;
    uint64_t tf_esr;
    uint64_t tf_far;
    uint64_t tf_x[30];
} __attribute__((__packed__)) FGdbTrapframe;

static inline void dbg_enable(void)
{
    __asm __volatile("msr daifclr, #(" __XSTRING(DAIF_D) ")");
}

static inline size_t gdb_cpu_regsz(int regnum)
{
    if (regnum == GDB_REG_CSPR || regnum == GDB_REG_FPSR || regnum == GDB_REG_FPCR)
    {
        return (4);
    }
    else if (regnum >= GDB_REG_V0 && regnum <= GDB_REG_V31)
    {
        return (16);
    }

    return (8);
}

static inline int gdb_cpu_query(void)
{
    return (0);
}

static inline void *gdb_begin_write(void)
{
    return (NULL);
}

static inline void gdb_end_write(void *arg)
{
}

void *gdb_cpu_getreg(FGdb *const, int, size_t *);
void gdb_cpu_setreg(FGdb *const, int, void *);
int gdb_cpu_signal(FGdb *const, int, int);
void gdb_cpu_stop_reason(FGdb *const, int, int);

void kdb_cpu_clear_singlestep(FGdb *const);
void kdb_cpu_set_singlestep(FGdb *const);
int kdb_cpu_set_watchpoint(FGdb *const, uintptr_t addr, size_t size, int access);
int kdb_cpu_clr_watchpoint(FGdb *const, uintptr_t addr, size_t size);

int kdb_cpu_set_hwbreakpoint(FGdb *const gdb, uintptr_t addr, size_t size);
int kdb_cpu_clr_hwbreakpoint(FGdb *const gdb, uintptr_t addr, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* FGDB_ARCH_H */