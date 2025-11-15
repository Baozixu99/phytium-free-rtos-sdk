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
 * FilePath: fcan_hw.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:28:50
 * Description:  This files is for the can register related functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2022/5/26  first release
 */


#include "fcan_hw.h"
#include "fparameters.h"
#include "fassert.h"
#include "fdrivers_port.h"

#define CAN_HW_DEBUG_TAG "CAN_HW"
#define FCAN_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(CAN_HW_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_INFO(format, ...) FT_DEBUG_PRINT_I(CAN_HW_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_WARN(format, ...) FT_DEBUG_PRINT_W(CAN_HW_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(CAN_HW_DEBUG_TAG, format, ##__VA_ARGS__)


void FCanDump(uintptr base_addr)
{

    FCAN_DEBUG("Off[0x%x]: FCAN_CTRL_OFFSET  = 0x%08x\r\n", base_addr + FCAN_CTRL_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_CTRL_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_INTR_OFFSET   = 0x%08x\r\n", base_addr + FCAN_INTR_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_INTR_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_ARB_RATE_CTRL_OFFSET    = 0x%08x\r\n", base_addr + FCAN_ARB_RATE_CTRL_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_ARB_RATE_CTRL_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_DAT_RATE_CTRL_OFFSET   = 0x%08x\r\n", base_addr + FCAN_DAT_RATE_CTRL_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_DAT_RATE_CTRL_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_ACC_ID0_OFFSET    = 0x%08x\r\n", base_addr + FCAN_ACC_ID0_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_ACC_ID0_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_ACC_ID1_OFFSET = 0x%08x\r\n", base_addr + FCAN_ACC_ID1_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_ACC_ID1_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_ACC_ID2_OFFSET = 0x%08x\r\n", base_addr + FCAN_ACC_ID2_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_ACC_ID2_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_ACC_ID3_OFFSET  = 0x%08x\r\n", base_addr + FCAN_ACC_ID3_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_ACC_ID3_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_ACC_ID0_MASK_OFFSET    = 0x%08x\r\n", base_addr + FCAN_ACC_ID0_MASK_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_ACC_ID0_MASK_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_ACC_ID1_MASK_OFFSET   = 0x%08x\r\n", base_addr + FCAN_ACC_ID1_MASK_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_ACC_ID1_MASK_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_ACC_ID2_MASK_OFFSET    = 0x%08x\r\n", base_addr + FCAN_ACC_ID2_MASK_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_ACC_ID2_MASK_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_ACC_ID3_MASK_OFFSET = 0x%08x\r\n", base_addr + FCAN_ACC_ID3_MASK_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_ACC_ID3_MASK_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_XFER_STS_OFFSET = 0x%08x\r\n", base_addr + FCAN_XFER_STS_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_XFER_STS_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_ERR_CNT_OFFSET  = 0x%08x\r\n", base_addr + FCAN_ERR_CNT_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_ERR_CNT_OFFSET));

    FCAN_DEBUG("Off[0x%x]: FCAN_FIFO_CNT_OFFSET = 0x%08x\r\n", base_addr + FCAN_FIFO_CNT_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_FIFO_CNT_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_DMA_CTRL_OFFSET = 0x%08x\r\n", base_addr + FCAN_DMA_CTRL_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_DMA_CTRL_OFFSET));

    FCAN_DEBUG("Off[0x%x]: FCAN_XFER_EN_OFFSET = 0x%08x\r\n", base_addr + FCAN_XFER_EN_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_XFER_EN_OFFSET));

    FCAN_DEBUG("Off[0x%x]: FCAN_FRM_INFO_OFFSET  = 0x%08x\r\n", base_addr + FCAN_FRM_INFO_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_FRM_INFO_OFFSET));

    FCAN_DEBUG("Off[0x%x]: FCAN_TX_FIFO_OFFSET  = 0x%08x\r\n", base_addr + FCAN_TX_FIFO_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_TX_FIFO_OFFSET));
    FCAN_DEBUG("Off[0x%x]: FCAN_RX_FIFO_OFFSET  = 0x%08x\r\n", base_addr + FCAN_RX_FIFO_OFFSET,
               FCAN_READ_REG32(base_addr, FCAN_RX_FIFO_OFFSET));

    FCAN_DEBUG("\r\n");
}