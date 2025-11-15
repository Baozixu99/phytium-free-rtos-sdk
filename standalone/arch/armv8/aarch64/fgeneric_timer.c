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
 * FilePath: fgeneric_timer.c
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:32:59
 * Description:  This file provides the common helper routines for the generic timer API's 
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  huanghe	2021/11/13		initialization
 *  1.1  zhugengyu 2022/6/3     add tick api
 *  1.2  wangxiaodong 2023/05/29  modify api
 */

#include "fparameters.h"
#include "fgeneric_timer.h"
#include "faarch.h"
#include "fassert.h"
#include "fkernel.h"

#define CTL_ENABLE_MASK    BIT(0)
#define CTL_INTERRUPT_MASK BIT(1)

/**
 * @name: GenericTimerSetTimerCompareValue
 * @msg:  Set generic timer CompareValue
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @param {u64} timeout, timeout value
 * @return {void}
 */
void GenericTimerSetTimerCompareValue(u32 id, u64 timeout)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1),
                "Please use correct int id");

    if (id == GENERIC_TIMER_ID0)
    {
        AARCH64_WRITE_SYSREG(cntp_cval_el0, timeout);
    }
    else
    {
        AARCH64_WRITE_SYSREG(cntv_cval_el0, timeout);
    }
}

/**
 * @name: GenericTimerSetTimerValue
 * @msg:  Set generic timer TimerValue
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @param {u64} timeout, timeout value
 * @return {void}
 */
void GenericTimerSetTimerValue(u32 id, u32 timeout)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1),
                "Please use correct int id");

    if (id == GENERIC_TIMER_ID0)
    {
        AARCH64_WRITE_SYSREG(cntp_tval_el0, timeout);
    }
    else
    {
        AARCH64_WRITE_SYSREG(cntv_tval_el0, timeout);
    }
}

/**
 * @name: GenericTimerInterruptEnable
 * @msg:  Unmask generic timer interrupt
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @return {void}
 */
void GenericTimerInterruptEnable(u32 id)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1),
                "Please use correct int id");
    u64 ctrl = 0;
    if (id == GENERIC_TIMER_ID0)
    {
        ctrl = AARCH64_READ_SYSREG(cntp_ctl_el0);
        ctrl &= (~CTL_INTERRUPT_MASK);
        AARCH64_WRITE_SYSREG(cntp_ctl_el0, ctrl);
    }
    else
    {
        ctrl = AARCH64_READ_SYSREG(cntv_ctl_el0);
        ctrl &= (~CTL_INTERRUPT_MASK);
        AARCH64_WRITE_SYSREG(cntv_ctl_el0, ctrl);
    }
}

/**
 * @name: GenericTimerInterruptDisable
 * @msg:  Mask generic timer interrupt
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @return {void}
 */
void GenericTimerInterruptDisable(u32 id)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1),
                "Please use correct int id");
    u64 ctrl = 0;
    if (id == GENERIC_TIMER_ID0)
    {
        ctrl = AARCH64_READ_SYSREG(cntp_ctl_el0);
        ctrl |= CTL_INTERRUPT_MASK;
        AARCH64_WRITE_SYSREG(cntp_ctl_el0, ctrl);
    }
    else
    {
        ctrl = AARCH64_READ_SYSREG(cntv_ctl_el0);
        ctrl |= CTL_INTERRUPT_MASK;
        AARCH64_WRITE_SYSREG(cntv_ctl_el0, ctrl);
    }
}

/**
 * @name: GenericTimerStart
 * @msg:  Enable generic timer
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @return {void}
 */
void GenericTimerStart(u32 id)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1),
                "Please use correct int id");
    u64 ctrl = 0;
    if (id == GENERIC_TIMER_ID0)
    {
        ctrl = AARCH64_READ_SYSREG(cntp_ctl_el0);
        ctrl |= CTL_ENABLE_MASK;
        AARCH64_WRITE_SYSREG(cntp_ctl_el0, ctrl);
    }
    else
    {
        ctrl = AARCH64_READ_SYSREG(cntv_ctl_el0);
        ctrl |= CTL_ENABLE_MASK;
        AARCH64_WRITE_SYSREG(cntv_ctl_el0, ctrl);
    }
}

/**
 * @name: GenericTimerStop
 * @msg:  Disable generic timer
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @return {void}
 */
void GenericTimerStop(u32 id)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1),
                "Please use correct generic timer id");
    u64 ctrl = 0;
    if (id == GENERIC_TIMER_ID0)
    {
        ctrl = AARCH64_READ_SYSREG(cntp_ctl_el0);
        ctrl &= (~CTL_ENABLE_MASK);
        AARCH64_WRITE_SYSREG(cntp_ctl_el0, ctrl);
    }
    else
    {
        ctrl = AARCH64_READ_SYSREG(cntv_ctl_el0);
        ctrl &= (~CTL_ENABLE_MASK);
        AARCH64_WRITE_SYSREG(cntv_ctl_el0, ctrl);
    }
}

/**
 * @name: GenericTimerFrequecy
 * @msg:  Get generic timer frequency of the system counter
 * @param {null} 
 * @return {u64} frequency of the system counter
 */
u64 GenericTimerFrequecy(void)
{
    u64 rate = AARCH64_READ_SYSREG(cntfrq_el0);
    return rate;
}

/**
 * @name: GenericTimerRead
 * @msg:  Get generic timer count value
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @return {u64} count value
 */
u64 GenericTimerRead(u32 id)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1),
                "Please use correct generic timer id");

    if (id == GENERIC_TIMER_ID0)
    {
        return AARCH64_READ_SYSREG(cntpct_el0);
    }
    else
    {
        return AARCH64_READ_SYSREG(cntvct_el0);
    }
}
