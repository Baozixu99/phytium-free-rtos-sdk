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
 * FilePath: fmhu_hw.h
 * Date: 2022-12-29 16:40:54
 * LastEditTime: 2022-12-29 16:40:55
 * Description:  This file is for mhu hardware define
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2023/12/30 init
 */
#ifndef FMHU_HW_H
#define FMHU_HW_H

#include "fio.h"
#include "ftypes.h"
#include "ferror_code.h"
#include "fassert.h"
#include "fdrivers_port.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SE_OS_STAT_OFFSET                 0x0
#define SE_OS_CLR_OFFSET                  0x10

#define AP_OS_STAT_OFFSET                 0x100
#define AP_OS_SET_OFFSET                  0x108
#define AP_OS_CLR_OFFSET                  0x110

#define AP_OS_SET                         0x1

#define FMHU_DATA_MASK                    GENMASK(30, 0)

#define FMHU_READ_REG32(addr, reg_offset) FtIn32((addr) + (u32)(reg_offset))
#define FMHU_WRITE_REG32(addr, reg_offset, reg_value) \
    FtOut32((addr) + (u32)(reg_offset), (u32)(reg_value))

/***************** Macros (Inline Functions) Definitions *********************/

#define FMHU_READ_STAT(addr) \
    (u32)(FMHU_DATA_MASK & FMHU_READ_REG32(addr, AP_OS_STAT_OFFSET))

#define FMHU_READ_SECU(addr) \
    (u32)((u32)(1U << 31) & FMHU_READ_REG32(addr, AP_OS_STAT_OFFSET))

#define FMHU_WRITE_SET(addr, dat)           FMHU_WRITE_REG32((addr), AP_OS_SET_OFFSET, (dat))

#define FMHU_WRITE_CLR(addr, dat)           FMHU_WRITE_REG32((addr), AP_OS_CLR_OFFSET, (dat))

#define FMHU_WRITE_INT_MASK(irq_addr, dat)  FtOut32((irq_addr), (dat))

#define FMHU_READ_INT_MASK(irq_addr)        FtIn32(irq_addr)

#define FMHU_READ_SE_STAT(addr) \
    (u32)(FMHU_DATA_MASK & FMHU_READ_REG32(addr, SE_OS_STAT_OFFSET))

#define FMHU_WRITE_SE_CLR(addr, dat) FMHU_WRITE_REG32(addr, SE_OS_CLR_OFFSET, (dat))
/************************** Function Prototypes ******************************/

/**
 * @name: FMhuGetChanStatus
 * @msg: Mhu get channel status
 * @return {*}
 * @param {uintptr} addr
 */
static inline u32 FMhuGetChanStatus(uintptr addr)
{
    FASSERT(addr);

    return FMHU_READ_STAT(addr);
}

/**
 * @name: FMhuSendData
 * @msg: Mhu write the AP_OS_SET register to send share memory Data
 * @return {*}
 * @param {uintptr} addr
 * @param {void *} data
 */
static inline void FMhuSendData(uintptr addr, void *data)
{
    FASSERT(addr);
    FASSERT(data);

    u32 *data_ptr = (u32 *)data;
    FMHU_WRITE_SET(addr, *data_ptr);
    return;
}

/**
 * @name: FMhuStartup
 * @msg: 
 * @return {*}
 * @param {uintptr} addr
 */
static inline void FMhuStartup(uintptr addr)
{
    FASSERT(addr);

    u32 val;

    val = FMHU_READ_STAT(addr);
    FMHU_WRITE_CLR(addr, val);
    /* irq set */

    return;
}

/**
 * @name: FMhuShutdown
 * @msg: 
 * @return {*}
 * @param {uintptr} irq_addr
 */
static inline void FMhuShutdown(uintptr irq_addr)
{
    FASSERT(irq_addr);

    FMHU_WRITE_INT_MASK(irq_addr, 0);
    return;
}

/**
 * @name: FMhuLastTxDone
 * @msg: 
 * @return {*}
 * @param {uintptr} addr
 */
static inline u32 FMhuLastTxDone(uintptr addr, boolean secu_flag)
{
    FASSERT(addr);

    return secu_flag ? (FMHU_READ_SECU(addr) == (u32)(1U << 31)) : (FMHU_READ_STAT(addr) == 0);
}

#ifdef __cplusplus
}
#endif

#endif