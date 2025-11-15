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
 * FilePath: frtc_hw.h
 * Date: 2021-08-25 14:53:42
 * LastEditTime: 2021-08-26 09:01:54
 * Description:  This file is for rtc register definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong 2021/8/26 first commit
 */

#ifndef DRIVERS_RTC_FRTC_HW_H
#define DRIVERS_RTC_FRTC_HW_H

#include "ftypes.h"
#include "fio.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Rtc register definitions */

#define FRTC_CMR                          0x04
#define FRTC_AES_SEL                      0x08
#define FRTC_AES_SEL_COUNTER              0x100

#define FRTC_CCR                          0x0C
#define FRTC_STAT                         0x10
#define FRTC_RSTAT                        0x14
#define FRTC_EOI                          0x18
#define FRTC_VER                          0x1C
#define FRTC_CDR_LOW                      0x20
#define FRTC_CCVR                         0x24
#define FRTC_CLR_LOW                      0x28
#define FRTC_CLR                          0x2c

#define FRTC_COUNTER_HB_OFFSET            15
#define FRTC_COUNTER_LB_MASK              0x7fff
#define FRTC_COUNTER_DELAY                150

/***************** Macros (Inline Functions) Definitions *********************/

/**
 * @name: RTC_READ_REG32
 * @msg:  读取RTC寄存器
 * @param {u32} addr 定时器的基地址
 * @param {u32} reg_offset   定时器的寄存器的偏移
 * @return {u32} 寄存器参数
 */
#define FRTC_READ_REG32(addr, reg_offset) FtIn32((addr) + (u32)reg_offset)

/**
 * @name: RTC_READ_REG64
 * @msg:  读取RTC寄存器
 * @param {u32} addr 定时器的基地址
 * @param {u32} reg_offset   定时器的寄存器的偏移
 * @return {u64} 寄存器参数
 */
#define FRTC_READ_REG64(addr, reg_offset) FtIn64((addr) + (u64)reg_offset)

/**
 * @name: RTC_WRITE_REG32
 * @msg:  写入RTC寄存器
 * @param {u32} addr 定时器的基地址
 * @param {u32} reg_offset   定时器的寄存器的偏移
 * @param {u32} reg_value    写入寄存器参数
 * @return {void}
 */
#define FRTC_WRITE_REG32(addr, reg_offset, reg_value) \
    FtOut32((addr) + (u32)reg_offset, (u32)reg_value)

#define FRTC_WRITE_AES_SEL(addr, regVal) \
    FRTC_WRITE_REG32((addr), FRTC_AES_SEL, (regVal))
#define FRTC_WRITE_CLR_LOW(addr, regVal) \
    FRTC_WRITE_REG32((addr), FRTC_CLR_LOW, (u32)(regVal))
#define FRTC_WRITE_CLR(addr, regVal) FRTC_WRITE_REG32((addr), FRTC_CLR, (u32)(regVal))

#define FRTC_READ_CDR_LOW(addr)      FRTC_READ_REG32((addr), FRTC_CDR_LOW)
#define FRTC_READ_CCVR(addr)         FRTC_READ_REG32((addr), FRTC_CCVR)


#ifdef __cplusplus
}
#endif

#endif