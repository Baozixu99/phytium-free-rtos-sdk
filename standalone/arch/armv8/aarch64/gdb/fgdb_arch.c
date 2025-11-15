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
 * Modified from FreeBSD sys/arm64/arm64/debug_monitor.c with LICENSE
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
 * FilePath: fgdb_arch.c
 * Created Date: 2024-08-27 17:05:14
 * Last Modified: 2024-08-27 17:34:17
 * Description:  This file is for gdb aarch64 function implmentation
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0   zhugengyu   2025/8/12    init commit
 */

/***************************** Include Files *********************************/
#include "fdebug.h"
#include "fassert.h"
#include "faarch.h"
#include "fprintk.h"
#include "fgdb.h"
#include "fgdb_def.h"
#include "fgdb_arch.h"

/************************** Constant Definitions *****************************/
#define EBUSY                   16 /* Device or resource busy */
#define EINVAL                  22 /* Invalid argument */
#define ENOSPC                  28 /* No space left on device */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FGDB_DEBUG_TAG          "FGDB"
#define FGDB_ERROR(format, ...) FT_DEBUG_PRINT_E(FGDB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDB_WARN(format, ...)  FT_DEBUG_PRINT_W(FGDB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDB_INFO(format, ...)  FT_DEBUG_PRINT_I(FGDB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDB_DEBUG(format, ...) FT_DEBUG_PRINT_D(FGDB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDB_RAW(format, ...)   FT_RAW_PRINTF(format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
void dbg_register_sync(FGdb *const gdb);
/*****************************************************************************/

void *gdb_cpu_getreg(FGdb *const gdb, int regnum, size_t *regsz)
{

    *regsz = gdb_cpu_regsz(regnum);

    switch (regnum)
    {
        case GDB_REG_LR:
            return (&gdb->gdb_frame->tf_lr);
        case GDB_REG_SP:
            return (&gdb->gdb_frame->tf_sp);
        case GDB_REG_PC:
            return (&gdb->gdb_frame->tf_elr);
        case GDB_REG_CSPR:
            return (&gdb->gdb_frame->tf_spsr);
        default:
            if (regnum >= GDB_REG_X0 && regnum <= GDB_REG_X29)
            {
                return (&gdb->gdb_frame->tf_x[regnum - GDB_REG_X0]);
            }
            break;
    }

    return (NULL);
}

void gdb_cpu_setreg(FGdb *const gdb, int regnum, void *val)
{
    u64 regval = *(u64 *)val;

    /* For curthread, keep the pcb and trapframe in sync. */
    switch (regnum)
    {
        case GDB_REG_PC:
            gdb->gdb_frame->tf_elr = regval;
            break;
        case GDB_REG_SP:
            gdb->gdb_frame->tf_sp = regval;
            break;
        default:
            if (regnum >= GDB_REG_X0 && regnum <= GDB_REG_X29)
            {
                gdb->gdb_frame->tf_x[regnum] = regval;
            }
            break;
    }
}

int gdb_cpu_signal(FGdb *const gdb, int type, int code)
{
    switch (type)
    {
        case EXCP_WATCHPT_EL1:
        case EXCP_SOFTSTP_EL1:
        case EXCP_BRKPT_EL1:
        case EXCP_BRK:
            return (SIGTRAP);
    }
    return (SIGEMT);
}

void gdb_cpu_stop_reason(FGdb *const gdb, int type, int code)
{

    if (type == EXCP_WATCHPT_EL1)
    {
        gdb_tx_str(gdb, "watch:");
        gdb_tx_varhex(gdb, (uintmax_t)READ_SPECIALREG(far_el1));
        gdb_tx_char(gdb, ';');
    }
}

void dbg_wb_write_reg(int reg, int n, uint64_t val)
{
    switch (reg + n)
    {
        SWITCH_CASES_WRITE_WB_REG(DBG_WB_WVR, DBG_REG_BASE_WVR, val);
        SWITCH_CASES_WRITE_WB_REG(DBG_WB_WCR, DBG_REG_BASE_WCR, val);
        SWITCH_CASES_WRITE_WB_REG(DBG_WB_BVR, DBG_REG_BASE_BVR, val);
        SWITCH_CASES_WRITE_WB_REG(DBG_WB_BCR, DBG_REG_BASE_BCR, val);
        default:
            FGDB_ERROR("trying to write to wrong debug register %d", n);
            return;
    }
    ISB();
}

static int dbg_find_free_slot(FGdb *const gdb, FGdbBreakType type)
{
    uint64_t *reg;
    int max, i = -1;

    switch (type)
    {
        case DBG_TYPE_BREAKPOINT:
            max = gdb->dbg_breakpoint_num;
            reg = gdb->monitor.dbg_bcr;
            break;
        case DBG_TYPE_WATCHPOINT:
            max = gdb->dbg_watchpoint_num;
            reg = gdb->monitor.dbg_wcr;
            break;
        default:
            FGDB_ERROR("Unsupported debug type");
            return (i);
    }

    for (i = 0; i < max; i++)
    {
        if ((reg[i] & DBG_WB_CTRL_E) == 0)
        {
            return (i);
        }
    }

    return (-1);
}

static int dbg_find_slot(FGdb *const gdb, FGdbBreakType type, uintptr_t addr)
{
    uint64_t *reg_addr, *reg_ctrl;
    int max, i = -1;

    switch (type)
    {
        case DBG_TYPE_BREAKPOINT:
            max = gdb->dbg_breakpoint_num;
            reg_addr = gdb->monitor.dbg_bvr;
            reg_ctrl = gdb->monitor.dbg_bcr;
            break;
        case DBG_TYPE_WATCHPOINT:
            max = gdb->dbg_watchpoint_num;
            reg_addr = gdb->monitor.dbg_wvr;
            reg_ctrl = gdb->monitor.dbg_wcr;
            break;
        default:
            FGDB_ERROR("Unsupported debug type");
            return (i);
    }

    for (i = 0; i < max; i++)
    {
        if (reg_addr[i] == addr && (reg_ctrl[i] & DBG_WB_CTRL_E) != 0)
        {
            return (i);
        }
    }

    return (-1);
}

static int dbg_setup_watchpoint(FGdb *const gdb, uintptr_t addr, size_t size, FGdbDebugAccessType access)
{
    uint64_t wcr_size, wcr_priv, wcr_access;
    int i;

    i = dbg_find_free_slot(gdb, DBG_TYPE_WATCHPOINT);
    if (i == -1)
    {
        FGDB_ERROR("Can not find slot for watchpoint, max %d"
                   " watchpoints supported",
                   gdb->dbg_watchpoint_num);
        return (EBUSY);
    }

    switch (size)
    {
        case 1:
            wcr_size = DBG_WATCH_CTRL_LEN_1;
            break;
        case 2:
            wcr_size = DBG_WATCH_CTRL_LEN_2;
            break;
        case 4:
            wcr_size = DBG_WATCH_CTRL_LEN_4;
            break;
        case 8:
            wcr_size = DBG_WATCH_CTRL_LEN_8;
            break;
        default:
            FGDB_ERROR("Unsupported address size for watchpoint: %zu", size);
            return (EINVAL);
    }

    wcr_priv = DBG_WB_CTRL_EL1;

    switch (access)
    {
        case HW_BREAKPOINT_X:
            wcr_access = DBG_WATCH_CTRL_EXEC;
            break;
        case HW_BREAKPOINT_R:
            wcr_access = DBG_WATCH_CTRL_LOAD;
            break;
        case HW_BREAKPOINT_W:
            wcr_access = DBG_WATCH_CTRL_STORE;
            break;
        case HW_BREAKPOINT_RW:
            wcr_access = DBG_WATCH_CTRL_LOAD | DBG_WATCH_CTRL_STORE;
            break;
        default:
            FGDB_RAW("Unsupported access type for watchpoint: %d\n", access);
            return (EINVAL);
    }

    gdb->monitor.dbg_wvr[i] = addr;
    gdb->monitor.dbg_wcr[i] = wcr_size | wcr_access | wcr_priv | DBG_WB_CTRL_E;
    gdb->monitor.dbg_enable_count++;
    gdb->monitor.dbg_flags |= DBGMON_ENABLED;

    dbg_register_sync(gdb);
    return (0);
}

static int dbg_remove_watchpoint(FGdb *const gdb, uintptr_t addr, size_t size)
{
    int i;

    i = dbg_find_slot(gdb, DBG_TYPE_WATCHPOINT, addr);
    if (i == -1)
    {
        FGDB_ERROR("Can not find watchpoint for address 0%lx", addr);
        return (EINVAL);
    }

    gdb->monitor.dbg_wvr[i] = 0;
    gdb->monitor.dbg_wcr[i] = 0;
    gdb->monitor.dbg_enable_count--;
    if (gdb->monitor.dbg_enable_count == 0)
    {
        gdb->monitor.dbg_flags &= ~DBGMON_ENABLED;
    }

    dbg_register_sync(gdb);
    return (0);
}

void dbg_register_sync(FGdb *const gdb)
{
    uint64_t mdscr;
    int i;

    for (i = 0; i < gdb->dbg_breakpoint_num; i++)
    {
        dbg_wb_write_reg(DBG_REG_BASE_BCR, i, gdb->monitor.dbg_bcr[i]);
        dbg_wb_write_reg(DBG_REG_BASE_BVR, i, gdb->monitor.dbg_bvr[i]);
    }

    for (i = 0; i < gdb->dbg_watchpoint_num; i++)
    {
        dbg_wb_write_reg(DBG_REG_BASE_WCR, i, gdb->monitor.dbg_wcr[i]);
        dbg_wb_write_reg(DBG_REG_BASE_WVR, i, gdb->monitor.dbg_wvr[i]);
    }

    mdscr = READ_SPECIALREG(mdscr_el1);
    if ((gdb->monitor.dbg_flags & DBGMON_ENABLED) == 0)
    {
        mdscr &= ~(MDSCR_MDE | MDSCR_KDE);
    }
    else
    {
        mdscr |= MDSCR_MDE;
        if ((gdb->monitor.dbg_flags & DBGMON_KERNEL) == DBGMON_KERNEL)
        {
            mdscr |= MDSCR_KDE;
        }
    }
    WRITE_SPECIALREG(mdscr_el1, mdscr);
    ISB();
}

void kdb_cpu_set_singlestep(FGdb *const gdb)
{

    FASSERT_MSG((READ_SPECIALREG(daif) & PSR_D) == PSR_D,
                "%s: debug exceptions are not masked", __func__);
    FASSERT(gdb->gdb_frame);

    if (gdb->compiled_brk_process)
    {
        gdb->gdb_frame->tf_elr += BREAK_INSTR_SIZE;
        gdb->compiled_brk_process = FALSE;
    }

    gdb->gdb_frame->tf_spsr |= PSR_SS;

    /*
	 * TODO: Handle single stepping over instructions that access
	 * the DAIF values. On a read the value will be incorrect.
	 */
    gdb->monitor.dbg_flags &= ~PSR_DAIF;
    gdb->monitor.dbg_flags |= gdb->gdb_frame->tf_spsr & PSR_DAIF;
    gdb->gdb_frame->tf_spsr |= (PSR_A | PSR_I | PSR_F);

    WRITE_SPECIALREG(mdscr_el1, READ_SPECIALREG(mdscr_el1) | MDSCR_SS | MDSCR_KDE);

    /*
	 * Disable breakpoints and watchpoints, e.g. stepping
	 * over watched instruction will trigger break exception instead of
	 * single-step exception and locks CPU on that instruction for ever.
	 */
    if ((gdb->monitor.dbg_flags & DBGMON_ENABLED) != 0)
    {
        WRITE_SPECIALREG(mdscr_el1, READ_SPECIALREG(mdscr_el1) & ~MDSCR_MDE);
    }
}

void kdb_cpu_clear_singlestep(FGdb *const gdb)
{

    FASSERT_MSG((READ_SPECIALREG(daif) & PSR_D) == PSR_D,
                "%s: debug exceptions are not masked", __func__);
    FASSERT(gdb->gdb_frame);

    if (gdb->compiled_brk_process)
    {
        gdb->gdb_frame->tf_elr += BREAK_INSTR_SIZE;
        gdb->compiled_brk_process = FALSE;
    }

    gdb->gdb_frame->tf_spsr &= ~PSR_DAIF;
    gdb->gdb_frame->tf_spsr |= gdb->monitor.dbg_flags & PSR_DAIF;

    WRITE_SPECIALREG(mdscr_el1, READ_SPECIALREG(mdscr_el1) & ~(MDSCR_SS | MDSCR_KDE));

    /* Restore breakpoints and watchpoints */
    if ((gdb->monitor.dbg_flags & DBGMON_ENABLED) != 0)
    {
        WRITE_SPECIALREG(mdscr_el1, READ_SPECIALREG(mdscr_el1) | MDSCR_MDE);

        if ((gdb->monitor.dbg_flags & DBGMON_KERNEL) != 0)
        {
            WRITE_SPECIALREG(mdscr_el1, READ_SPECIALREG(mdscr_el1) | MDSCR_KDE);
        }
    }
}

int kdb_cpu_set_watchpoint(FGdb *const gdb, uintptr_t addr, size_t size, int access)
{
    FGdbDebugAccessType dbg_access;

    switch (access)
    {
        case KDB_DBG_ACCESS_R:
            dbg_access = HW_BREAKPOINT_R;
            break;
        case KDB_DBG_ACCESS_W:
            dbg_access = HW_BREAKPOINT_W;
            break;
        case KDB_DBG_ACCESS_RW:
            dbg_access = HW_BREAKPOINT_RW;
            break;
        default:
            return (EINVAL);
    }

    return (dbg_setup_watchpoint(gdb, addr, size, dbg_access));
}

int kdb_cpu_clr_watchpoint(FGdb *const gdb, uintptr_t addr, size_t size)
{

    return (dbg_remove_watchpoint(gdb, addr, size));
}

int kdb_cpu_set_hwbreakpoint(FGdb *const gdb, uintptr_t addr, size_t size)
{
    uint64_t bcr_size, bcr_priv;
    int i;

    i = dbg_find_free_slot(gdb, DBG_TYPE_BREAKPOINT);
    if (i == -1)
    {
        FGDB_ERROR("Can not find slot for breakpoint, max %d"
                   " breakpoints supported",
                   gdb->dbg_breakpoint_num);
        return (EBUSY);
    }

    switch (size)
    {
        case 1:
            bcr_size = DBG_WATCH_CTRL_LEN_1;
            break;
        case 2:
            bcr_size = DBG_WATCH_CTRL_LEN_2;
            break;
        case 4:
            bcr_size = DBG_WATCH_CTRL_LEN_4;
            break;
        case 8:
            bcr_size = DBG_WATCH_CTRL_LEN_8;
            break;
        default:
            FGDB_ERROR("Unsupported address size for breakpoint: %zu", size);
            return (EINVAL);
    }

    bcr_priv = DBG_WB_CTRL_EL1;

    gdb->monitor.dbg_bvr[i] = addr;
    /* E, bit[0]: Enable breakpoint n, 1 
	   PMC, bit[2:1]: Privilege mode control, EL1
	   BAS, bit[12:5]: Byte address select*/
    gdb->monitor.dbg_bcr[i] = bcr_size | bcr_priv | DBG_WB_CTRL_E;
    gdb->monitor.dbg_enable_count++;
    gdb->monitor.dbg_flags |= DBGMON_ENABLED;

    dbg_register_sync(gdb);
    return (0);
}

int kdb_cpu_clr_hwbreakpoint(FGdb *const gdb, uintptr_t addr, size_t size)
{
    int i;

    i = dbg_find_slot(gdb, DBG_TYPE_BREAKPOINT, addr);
    if (i == -1)
    {
        FGDB_ERROR("Can not find breakpoint for address 0%lx", addr);
        return (EINVAL);
    }

    gdb->monitor.dbg_bvr[i] = 0;
    gdb->monitor.dbg_bcr[i] = 0;
    gdb->monitor.dbg_enable_count--;
    if (gdb->monitor.dbg_enable_count == 0)
    {
        gdb->monitor.dbg_flags &= ~DBGMON_ENABLED;
    }

    dbg_register_sync(gdb);
    return (0);
}

static FGdb *gdb_instance = NULL;

void FGdbRegisterInstance(FGdb *const gdb)
{
    gdb_instance = gdb;
}

void FGdbDebugInterrupt(FGdbTrapframe *trap_frame)
{
    FASSERT(gdb_instance);
    FGdb *const gdb = gdb_instance;
    u64 esr, ec, iss;
    int code = GDB_STOPREASON_NONE;

    dbg_register_sync(gdb);

    esr = AARCH64_READ_SYSREG(esr_el1);
    ec = (esr >> 26) & 0x3f;

    iss = esr & ESR_ELx_BRK64_ISS_COMMENT_MASK;
    if (iss == GDB_COMPILED_DBG_BRK_IMM)
    {
        gdb->compiled_brk_process = TRUE;
    }

    switch (ec)
    {
        case EXCP_BRK:
        case EXCP_BRKPT_EL1:
            code = GDB_STOPREASON_BREAKPOINT;
            f_printk("Debug exception: breakpoint triggered ");
            break;
        case EXCP_WATCHPT_EL1:
            /* TODO: which kind of watchpoint hit */
            code = GDB_STOPREASON_WATCHPOINT_RW;
            f_printk("Debug exception: watchpoint triggered ");
            break;
        case EXCP_SOFTSTP_EL1:
            f_printk("Debug exception: step triggered ");
            code = GDB_STOPREASON_STEPPOINT;
            break;
    }

    f_printk("ec:0x%lx iss:0x%lx\r\n", ec, iss);

    gdb->gdb_frame = trap_frame;
    (void)FGdbTrap(gdb, ec, code);
    f_printk("Debug exception handled\r\n");
}