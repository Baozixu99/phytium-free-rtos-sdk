/*
 * Copyright (C) 2021, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fwdt_hw.h
 * Date: 2021-08-25 10:27:42
 * LastEditTime: 2022-02-25 11:44:33
 * Description:  This file is for wdt register definition.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2022/4/15   init commit
 */

#ifndef FWDT_HW_H
#define FWDT_HW_H

#include "fio.h"
#include "fdrivers_port.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Watchdog register definitions */

/* refresh frame */
#define FWDT_GWDT_WRR                     0x000
#define FWDT_GWDT_W_IIR                   0xfcc

/* control frame */
#define FWDT_GWDT_WCS                     0x000 /* WCS register */
#define FWDT_GWDT_WOR                     0x008
#define FWDT_GWDT_WCVL                    0x010
#define FWDT_GWDT_WCVH                    0x014

/* Watchdog Control and Status Register */
#define FWDT_GWDT_WCS_WDT_EN              BIT(0)
#define FWDT_GWDT_WCS_WS0                 BIT(1)
#define FWDT_GWDT_WCS_WS1                 BIT(2)

/***************** Macros (Inline Functions) Definitions *********************/

/**
 * @name: WDT_READ_REG32
 * @msg:  read WDT register
 * @param {u32} addr, base address
 * @param {u32} reg_offset, register offset
 * @return {u32} register value
 */
#define FWDT_READ_REG32(addr, reg_offset) FtIn32((addr) + (u32)(reg_offset))

/**
 * @name: FWDT_WRITE_REG32
 * @msg:  write WDT register
 * @param {u32} addr, base address
 * @param {u32} reg_offset, register offset
 * @param {u32} reg_value, value write to register
 * @return {u32} register value
 */
#define FWDT_WRITE_REG32(addr, reg_offset, reg_value) \
    FtOut32((addr) + (u32)(reg_offset), (u32)(reg_value))

#define FWDT_VERSION_MASK           GENMASK(19, 16)
#define FWDT_CONTINUATION_CODE_MASK GENMASK(11, 8)
#define FWDT_IDENTIFY_CODE_MASK     GENMASK(6, 0)
/**
 * @name: FWdtReadWCVH
 * @msg:  Read wdt wcvh register value. wcvl and wclh register stores the comparison value of the watchdog count.
 *          timeout value = comparison value - sys_cnt.
 * @param {uintptr} addr, pointer to a WdtCtrl base addr.
 * @return {u32} register value
 */

static inline u32 FWdtReadWCVH(uintptr addr)
{
    return FWDT_READ_REG32(addr, FWDT_GWDT_WCVH);
}

/**
 * @name: FWdtReadWCVL
 * @msg:  Read wdt wcvl register value. wcvl and wclh register stores the comparison value of the watchdog count.
 *          timeout value = comparison value - sys_cnt.
 * @param {uintptr} addr, pointer to a WdtCtrl base addr.
 * @return {u32} register value
 */
static inline u32 FWdtReadWCVL(uintptr addr)
{
    return FWDT_READ_REG32(addr, FWDT_GWDT_WCVL);
}

/**
 * @name: FWdtReadWOR
 * @msg:  Read wdt wor register value. used to set timeout value, wor + sys_cnt = wcv.
 * @param {uintptr} addr, pointer to a WdtCtrl base addr.
 * @return {u32} register value
 */
static inline u32 FWdtReadWOR(uintptr addr)
{
    return FWDT_READ_REG32(addr, FWDT_GWDT_WOR);
}

/**
 * @name: FWdtReadWCS
 * @msg:  Read wdt wcs register value. wcs is control and state register. bit0 enable(1) or disable(0) wdt.
 * @param {uintptr} addr, pointer to a WdtCtrl base addr.
 * @return {u32} register value
 */
static inline u32 FWdtReadWCS(uintptr addr)
{
    return FWDT_READ_REG32(addr, FWDT_GWDT_WCS);
}

void FWdtDump(uintptr base_addr);

#ifdef __cplusplus
}
#endif

#endif