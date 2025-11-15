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
 * FilePath: fi2s_msg_intr.c
 * Created Date: 2025-05-15 14:11:28
 * Last Modified: 2025-06-11 17:44:35
 * Description:  This file is for
 *
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

/***************************** Include Files *********************************/
#include <string.h>
#include "ftypes.h"
#include "fdrivers_port.h"
#include "fassert.h"
#include "fdebug.h"

#include "fmsg_common.h"
#include "fparameters.h"
#include "fi2s_msg_hw.h"
#include "fi2s_msg.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FI2S_MSG_DEBUG_TAG "FI2S-MSG_INTR"
#define FI2S_MSG_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FI2S_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2S_MSG_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FI2S_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2S_MSG_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FI2S_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2S_MSG_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FI2S_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
void FI2sMsgSetInterrupt(FI2sMsgCtrl *const instance, u32 is_enable)
{
    uintptr addr;
    addr = (uintptr)instance->config.dma_reg_base;
    u32 status = 0;
    status = FI2S_MSG_READ_REG32(addr, FI2S_DMA_STS);
    if (status & DMA_TX_DONE)
    {
        FI2S_MSG_WRITE_REG32(addr, FI2S_DMA_STS, DMA_TX_DONE);
    }

    if (status & DMA_RX_DONE)
    {
        FI2S_MSG_WRITE_REG32(addr, FI2S_DMA_STS, DMA_RX_DONE);
    }
    FI2S_MSG_WRITE_REG32(addr, FI2S_DMA_MASK_INT, 0x0);
}
