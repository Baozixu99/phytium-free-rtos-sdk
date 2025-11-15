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
 * FilePath: fi2c_msg_slave.c
 * Date: 2024-10-1 14:53:42
 * LastEditTime: 2024-10-10 08:36:14
 * Description:  This file is for msg i2c driver  file.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangzq      2024/10/08  first commit
 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fi2c_msg.h"
#include "fmsg_common.h"
#include "fi2c_msg_common.h"
#include "fi2c_msg_hw.h"
#include "fparameters.h"
#include "fdrivers_port.h"

/************************** Constant Definitions *****************************/
#define FI2C_MSG_RX_FIFO_FULL_ENABLE         (1)
#define FI2C_MSG_RX_FIFO_FULL_DISABLE        (0)
#define FI2C_MSG_CON_STOP_DET_IFADDR_ENABLE  (1)
#define FI2C_MSG_CON_STOP_DET_IFADDR_DISABLE (1)
#define FI2C_MSG_SLAVE_EVENT_MAX_CNT         (64)
#define FI2C_MSG_SLAVE_DATA_INDEX(index)     (index + 1)
#define FI2C_MSG_SLAVE_RX_INFO_SIZE          (2)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FI2C_MSG_DEBUG_TAG                   "I2C-MSG-SLAVE"
#define FI2C_MSG_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_MSG_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_MSG_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
static FError FI2cMsgSlaveVirtInit(FI2cMsgCtrl *const instance)
{
    FError ret = FI2C_MSG_SUCCESS;
    u8 slave_mode = FI2C_MSG_MASTER_MODE_FLAG;
    u8 rx_fifo_full = FI2C_MSG_RX_FIFO_FULL_DISABLE;
    u8 stop_det_ifaddr_en = FI2C_MSG_CON_STOP_DET_IFADDR_DISABLE;

    FI2cMsgVirtSetModuleEn(instance, FI2C_MSG_ADAPTER_MODULE_OFF);
    ret = FI2cMsgVirtCheckResult(instance);
    if (ret)
    {
        FI2C_MSG_ERROR("failed to set module off, ret = %d\n", ret);
        return ret;
    }

    if (instance->sda_hold_time)
    {
        FI2cMsgVirtSetSdaHold(instance, instance->sda_hold_time);
        ret = FI2cMsgVirtCheckResult(instance);
        if (ret != FI2C_MSG_SUCCESS)
        {
            FI2C_MSG_ERROR("failed to set sda hold, ret = %d\n", ret);
            return ret;
        }
    }
    if ((instance->mode & FI2C_MSG_CON_MASTER_MODE_MASK) == FI2C_MSG_SLAVE)
    {
        slave_mode = FI2C_MSG_SLAVE_MODE_FLAG;
    }
    else
    {
        FI2C_MSG_ERROR("I2c is not in slave mode");
        return ret;
    }

    FI2cMsgVirtSetCmd8(instance, FI2C_MSG_CMD_SET_MODE, slave_mode);
    ret = FI2cMsgVirtCheckResult(instance);
    if (ret != FI2C_MSG_SUCCESS)
    {
        FI2C_MSG_ERROR("failed to set mode, ret = %d\n", ret);
        return ret;
    }

    FI2cMsgVirtSetIntTl(instance, 0, 0);
    ret = FI2cMsgVirtCheckResult(instance);
    if (ret != FI2C_MSG_SUCCESS)
    {
        FI2C_MSG_ERROR("failed to set int tl, ret = %d\n", ret);
        return ret;
    }
    if ((instance->slave_cfg & FI2C_MSG_CON_RX_FIFO_FULL_HLD_MASK) == FI2C_MSG_CON_RX_FIFO_FULL_HLD_CTRL)
    {
        rx_fifo_full = FI2C_MSG_RX_FIFO_FULL_ENABLE;
    }

    FI2cMsgVirtSetCmd8(instance, FI2C_MSG_CMD_SET_RX_FIFO_FULL, rx_fifo_full);
    ret = FI2cMsgVirtCheckResult(instance);
    if (ret != FI2C_MSG_SUCCESS)
    {
        FI2C_MSG_ERROR("failed to set rx fifo full, ret = %d\n", ret);
        return ret;
    }
    if ((instance->slave_cfg & FI2C_MSG_STOP_DET_IFADDR_MASK) == FI2C_MSG_STOP_DET_IFADDRESSED)
    {
        stop_det_ifaddr_en = FI2C_MSG_CON_STOP_DET_IFADDR_ENABLE;
    }

    FI2cMsgVirtSetCmd8(instance, FI2C_MSG_CMD_SET_STOP_DET_IF_ADDRESSED, stop_det_ifaddr_en);
    ret = FI2cMsgVirtCheckResult(instance);
    if (ret != FI2C_MSG_SUCCESS)
    {
        FI2C_MSG_ERROR("failed to set stop det ifaddr, ret = %d\n", ret);
        return ret;
    }
    FI2cMsgSetInterrupt(instance, FI2C_MSG_ENABLE_INTERRUPT, FI2C_MSG_INTR_SLAVE_MASK);
    ret = FI2cMsgVirtCheckResult(instance);
    if (ret != FI2C_MSG_SUCCESS)
    {
        FI2C_MSG_ERROR("failed to set stop det ifaddr, ret = %d\n", ret);
        return ret;
    }

    return ret;
}

int FI2cMsgSlaveVirtProbe(FI2cMsgCtrl *const instance)
{
    int ret;
    instance->slave_cfg = FI2C_MSG_CON_RX_FIFO_FULL_HLD_CTRL | FI2C_MSG_CON_RESTART_EN |
                          FI2C_MSG_STOP_DET_IFADDRESSED;
    instance->mode = FI2C_MSG_SLAVE;
    instance->speed_mode = FI2C_STANDARD_SPEED;
    instance->clk_clock_frequency = FI2C_CLK_FREQ_HZ;

    FI2cMsgCommonRegfileIntrDisable(instance);
    FI2cMsgCommonRegfileS2MIntrClear(instance, 0);
    FI2cMsgCommonRegfileIntrEnable(instance);
    ret = FI2cMsgSlaveVirtInit(instance);
    if (ret != FI2C_MSG_SUCCESS)
    {
        return ret;
    }
    return ret;
}
