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
 * FilePath: fi2s_msg_hw.h
 * Created Date: 2025-05-14 14:40:19
 * Last Modified: 2025-06-11 17:39:45
 * Description:  This file is for
 *
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */
#ifndef FI2C_MSG_HW_H
#define FI2C_MSG_HW_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "fio.h"
#include "ferror_code.h"
#include "fmsg_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Type Definitions **********/

/***************************** Macro Definitions **********/
#define FI2S_MSG_READ_REG32(addr, reg_offset) FtIn32((addr) + (u32)(reg_offset))
#define FI2S_MSG_WRITE_REG32(addr, reg_offset, reg_value) \
    FtOut32((addr) + (u32)reg_offset, (u32)reg_value)

/* regfile */
#define SEND_INTR                  (1 << 4)

/* DMA register */
#define FI2S_DMA_CTL               0x0000
#define DMA_ENABLE                 0x1
#define FI2S_DMA_CHAL_CONFG0       0x0004
#define CHANNEL_0_1_ENABLE         0x8180
#define FI2S_DMA_STS               0x0008
#define DMA_TX_DONE                0x1
#define DMA_RX_DONE                0x100
#define FI2S_DMA_MASK_INT          0x000c
#define CHANNEL_0_1_INT_MASK       0x80000003
#define FI2S_DMA_BDLPU(x)          (0x40 * x + 0x0040)
#define FI2S_DMA_BDLPL(x)          (0x40 * x + 0x0044)
#define FI2S_DMA_CHALX_DEV_ADDR(x) (0x40 * x + 0x0048)
#define FI2S_LSD_BASE              0x28009000
#define PLAYBACK_ADDRESS_OFFSET    0x1c8
#define CAPTRUE_ADDRESS_OFFSET     0x1c0
#define FI2S_DMA_CHALX_LVI(x)      (0x40 * x + 0x004c)
#define FI2S_DMA_LPIB(x)           (0x40 * x + 0x0050)
#define FI2S_DMA_CHALX_CBL(x)      (0x40 * x + 0x0054)
#define FI2S_DMA_CHALX_CTL(x)      (0x40 * x + 0x0058)
#define PLAYBACK_START             0x1
#define CAPTURE_START              0x5
#define CTL_STOP                   0x0
#define FI2S_DMA_CHALX_DSIZE(x)    (0x40 * x + 0x0064)
#define BYTE_4                     0x0
#define BYTE_2                     0x2
#define D_SIZE(byte_mode, dir)     (byte_mode << (dir * 2))
#define FI2S_DMA_CHALX_DLENTH(x)   (0x40 * x + 0x0068)
#define D_LENTH                    0x0
/*********************register end ***************************/

#ifdef __cplusplus
}
#endif

#endif /* FI2S_MSG_HW_H */