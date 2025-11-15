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
 * LastEditTime: 2022-02-17 17:30:07
 * Description:  This file provides the common helper routines for the generic timer API's
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  huanghe	2021-11		initialization
 *  1.1  zhugengyu 2022/06/05     add tick api
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
        AARCH32_WRITE_SYSREG_64(CNTP_CVAL_64, timeout);
    }
    else
    {
        AARCH32_WRITE_SYSREG_64(CNTV_CVAL_64, timeout);
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
        AARCH32_WRITE_SYSREG_32(CNTP_TVAL, timeout);
    }
    else
    {
        AARCH32_WRITE_SYSREG_32(CNTV_TVAL, timeout);
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

    u32 ctrl = 0;
    if (id == GENERIC_TIMER_ID0)
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTP_CTL);
        ctrl &= (u32)(~CTL_INTERRUPT_MASK);
        AARCH32_WRITE_SYSREG_32(CNTP_CTL, ctrl);
    }
    else
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTV_CTL);
        ctrl &= (u32)(~CTL_INTERRUPT_MASK);
        AARCH32_WRITE_SYSREG_32(CNTV_CTL, ctrl);
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
    u32 ctrl = 0;
    if (id == GENERIC_TIMER_ID0)
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTP_CTL);
        ctrl |= (u32)CTL_INTERRUPT_MASK;
        AARCH32_WRITE_SYSREG_32(CNTP_CTL, ctrl);
    }
    else
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTV_CTL);
        ctrl |= (u32)CTL_INTERRUPT_MASK;
        AARCH32_WRITE_SYSREG_32(CNTV_CTL, ctrl);
    }
}

/**
 * @name: GenericTimerStart
 * @msg:  Enable generic timer, set this bit to 1 enables the timer output signal, 
 *          cooperate with GenericTimerInterruptEnable function generate interrupts.
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @return {void}
 */
void GenericTimerStart(u32 id)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1),
                "Please use correct int id");
    u32 ctrl = 0;
    if (id == GENERIC_TIMER_ID0)
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTP_CTL);
        ctrl |= (u32)CTL_ENABLE_MASK;
        AARCH32_WRITE_SYSREG_32(CNTP_CTL, ctrl);
    }
    else
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTV_CTL);
        ctrl |= (u32)CTL_ENABLE_MASK;
        AARCH32_WRITE_SYSREG_32(CNTV_CTL, ctrl);
    }
}

/**
 * @name: GenericTimerStop
 * @msg:  Disable generic timer, set this bit to 0 disables the timer output signal, 
 *          GenericTimerRead function is still available.
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @return {void}
 */
void GenericTimerStop(u32 id)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1),
                "Please use correct generic timer id");

    u32 ctrl = 0;
    if (id == GENERIC_TIMER_ID0)
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTP_CTL);
        ctrl &= (u32)(~CTL_ENABLE_MASK);
        AARCH32_WRITE_SYSREG_32(CNTP_CTL, ctrl);
    }
    else
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTV_CTL);
        ctrl &= (u32)(~CTL_ENABLE_MASK);
        AARCH32_WRITE_SYSREG_32(CNTV_CTL, ctrl);
    }
}

/**
 * @name: GenericTimerFrequecy
 * @msg:  Get generic timer frequency of the system counter
 * @param {null} 
 * @return {u32} frequency of the system counter
 */
u32 GenericTimerFrequecy(void)
{
    u32 rate = AARCH32_READ_SYSREG_32(CNTFRQ);
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
        ISB();
        return AARCH32_READ_SYSREG_64(CNTPCT_64);
    }
    else
    {
        ISB();
        return AARCH32_READ_SYSREG_64(CNTVCT_64);
    }
}
