/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fi2s_hw.h
 * Created Date: 2023-05-09 09:59:28
 * Last Modified: 2023-10-13 14:23:37
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/05/09  Modify the format and establish the version
 */

#ifndef FI2S_HW_H
#define FI2S_HW_H

/***************************** Include Files *********************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include "ftypes.h"
#include "ferror_code.h"
#include "fio.h"
#include "fi2s.h"
#include "fdrivers_port.h"

/****************************** Type Definitions *******************************/

#define FI2S_READ_REG32(addr, reg_offset) FtIn32((addr) + (u32)(reg_offset))
#define FI2S_WRITE_REG32(addr, reg_offset, reg_value) \
    FtOut32((addr) + (u32)reg_offset, (u32)reg_value)


/************************** Constant Definitions *****************************/

#define FI2S_IER                  0x000
#define FI2S_IRER                 0x004
#define FI2S_ITER                 0x008
#define FI2S_CER                  0x00C
#define FI2S_CCR                  0x010
#define FI2S_RXFFR                0x014
#define FI2S_TXFFR                0x018
#define FI2S_RRBR0(x)             (0x40 * x + 0x024)
#define FI2S_LRBR0(x)             (0x40 * x + 0x020)
#define FI2S_RER0(x)              (0x40 * x + 0x028)
#define FI2S_TER0(x)              (0x40 * x + 0x02C)
#define FI2S_RCR0(x)              (0x40 * x + 0x030)
#define FI2S_TCR0(x)              (0x40 * x + 0x034)
#define FI2S_ISR0(x)              (0x40 * x + 0x038)
#define FI2S_IMR0(x)              (0x40 * x + 0x03C)
#define FI2S_R0R0(x)              (0x40 * x + 0x040)
#define FI2S_TOR0(x)              (0x40 * x + 0x044)
#define FI2S_RFCR0(x)             (0x40 * x + 0x048)
#define FI2S_TFCR0(x)             (0x40 * x + 0x04C)
#define FI2S_RFF0(x)              (0x40 * x + 0x050)
#define FI2S_TFF0(x)              (0x40 * x + 0x054)
#define FI2S_RXDMA                0x01C0
#define FI2S_RRXDMA               0x01C4
#define FI2S_TXDMA                0x01C8
#define FI2S_RTXDMA               0x01CC
#define FI2S_FRAC_DIV             0x0C00
#define FI2S_EVEN_DIV             0x0C04


#define FI2S_INTR_TRANS_FO_MASK   BIT(5)
#define FI2S_INTR_TRANS_FE_MASK   BIT(4)
#define FI2S_INTR_RECRIVE_FO_MASK BIT(1)
#define FI2S_INTR_RECRIVE_FE_MASK BIT(0)

#define FI2S_IER_CTRL             BIT(0)
#define FI2S_CER_CTRL             BIT(0)


#define FI2S_RER0_CTRL            BIT(0)
#define FI2S_TER0_CTRL            BIT(0)
#define FI2S_RTXDMA_RESET         BIT(0)
#define FI2S_RRXDMA_RESET         BIT(0)
#define FI2S_TFF0_RESET           BIT(0)
#define FI2S_RFF0_RESET           BIT(0)
#define FI2S_RFF0_RESET           BIT(0)
#define FI2S_CER_CTRL             BIT(0)
#define FI2S_ITER_CTRL            BIT(0)
#define FI2S_TXFFR_RESET          BIT(0)
#define FI2S_RXFFR_RESET          BIT(0)
#define FI2S_IRER_CTRL            BIT(0)


#ifdef __cplusplus
}
#endif

#endif