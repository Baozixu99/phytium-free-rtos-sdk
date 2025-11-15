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
 * FilePath: fi2c_msg_common.c
 * Date: 2024-10-1 14:53:42
 * LastEditTime: 2024-10-10 08:36:14
 * Description:  This file is for I2C msg common configuration
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangzq     2024/10/1  first commit
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "ftypes.h"
#include "fi2c_msg.h"
#include "fparameters.h"
#include "fi2c_msg_common.h"
#include "fmsg_common.h"
#include "fi2c_msg_hw.h"
#include "fdrivers_port.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FI2C_MSG_DEBUG_TAG "I2C_MSG_OMMON"
#define FI2C_MSG_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_MSG_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_MSG_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
void FI2cMsgVirtSendMsg(FI2cMsgCtrl *const instance, FI2cMsgInfo *set_msg, boolean complete_flag)
{
    u64 *p_dest_msg = (u64 *)instance->tx_shmem_addr;
    u64 *p_src_msg = (u64 *)set_msg;
    int i, len;

    if (complete_flag)
    {
        instance->complete_flag = TRUE;
        instance->mng.is_need_check = FALSE;
    }

    len = DIV_ROUND_UP(instance->total_shmem_len, 8);

    for (i = 0; i < len; i++)
    {
        p_dest_msg[i] = p_src_msg[i];
    }
    /*For config cmd,no use tail and head,set this value to zero*/
    FI2C_MSG_WRITE_REG32(instance->config.msg.regfile, FMSG_TX_TAIL, 0);
    FI2C_MSG_WRITE_REG32(instance->config.msg.regfile, FMSG_TX_HEAD, 0);
    instance->mng.tx_cmd_cnt = 1;
    instance->mng.cur_cmd_cnt = 0;
    FI2cMsgTrigSlaveIntr(instance);
}

void FI2cMsgNotifySlave(FI2cMsgCtrl *const instance, boolean need_check)
{
    instance->mng.is_need_check = need_check;
    instance->complete_flag = TRUE;
    FI2cMsgTrigSlaveIntr(instance);
}

void FI2cMsgCommonSetCmd(FI2cMsgCtrl *const instance, FI2cMsgInfo *i2c_mng_msg, u8 cmd, u8 sub_cmd)
{
    i2c_mng_msg->cmd_type = cmd;
    i2c_mng_msg->cmd_subid = sub_cmd;
    i2c_mng_msg->status0 = FI2C_MSG_COMPLETE_UNKNOWN;
}

void FI2cMsgVirtSetCmdArray(FI2cMsgCtrl *const instance, u8 sub_cmd, u8 *data, u32 len)
{
    FI2cMsgInfo i2c_mng_msg;
    memset(&i2c_mng_msg, 0, sizeof(i2c_mng_msg));
    /*set cmd*/
    FI2cMsgCommonSetCmd(instance, &i2c_mng_msg, FMSG_CMD_DATA, sub_cmd);
    /*set data*/
    FI2cMsgMemcpy(&i2c_mng_msg.para[0], &data[0], len);
    /*set len*/
    instance->total_shmem_len = FI2C_MSG_CMDDATA_SIZE;
    FI2cMsgVirtSendMsg(instance, &i2c_mng_msg, FALSE);
}

void FI2cMsgVirtSetCmd8(FI2cMsgCtrl *const instance, u8 sub_cmd, u8 data)
{
    FI2cMsgInfo i2c_mng_msg;

    memset(&i2c_mng_msg, 0, sizeof(i2c_mng_msg));
    FI2cMsgCommonSetCmd(instance, &i2c_mng_msg, FMSG_CMD_SET, sub_cmd);
    i2c_mng_msg.para[0] = data;
    instance->total_shmem_len = FI2C_MSG_CMDDATA_SIZE;

    FI2cMsgVirtSendMsg(instance, &i2c_mng_msg, TRUE);
}

void FI2cMsgVirtSetCmd16(FI2cMsgCtrl *const instance, u8 sub_cmd, u16 data)
{
    FI2cMsgInfo i2c_mng_msg;
    u16 *cmd_data = (u16 *)&i2c_mng_msg.para[0];

    memset(&i2c_mng_msg, 0, sizeof(i2c_mng_msg));

    FI2cMsgCommonSetCmd(instance, &i2c_mng_msg, FMSG_CMD_SET, sub_cmd);
    *cmd_data = data;
    instance->total_shmem_len = FI2C_MSG_CMDDATA_SIZE;
    FI2cMsgVirtSendMsg(instance, &i2c_mng_msg, TRUE);
}

void FI2cMsgVirtSetCmd32(FI2cMsgCtrl *const instance, u8 sub_cmd, u32 data)
{
    FI2cMsgInfo i2c_mng_msg;
    u32 *cmd_data = (u32 *)&i2c_mng_msg.para[0];

    memset(&i2c_mng_msg, 0, sizeof(i2c_mng_msg));

    FI2cMsgCommonSetCmd(instance, &i2c_mng_msg, FMSG_CMD_SET, sub_cmd);
    *cmd_data = data;
    instance->total_shmem_len = FI2C_MSG_CMDDATA_SIZE;
    FI2cMsgVirtSendMsg(instance, &i2c_mng_msg, TRUE);
}

FError FI2cMsgVirtCheckResult(FI2cMsgCtrl *const instance)
{
    FError ret = FI2C_MSG_SUCCESS;
    FDriverMdelay(instance->timeout_ms);
    if (!instance->abort_source)
    {
        return ret;
    }
    else
    {
        instance->abort_source = BIT(FI2C_MSG_TIMEOUT);
        FI2C_MSG_ERROR("abort_source = %d\n", instance->abort_source);
        return FI2C_MSG_TIMEOUT;
    }
    return ret;
}

FError FI2cMsgVirtCheckStatus(FI2cMsgCtrl *const instance, FI2cMsgInfo *msg)
{
    FError result = FI2C_MSG_CHECK_STATUS_ERR;

    instance->abort_source = 1 << FI2C_MSG_CHECK_STATUS_ERR;

    if (msg->status0 != FI2C_MSG_COMPLETE_UNKNOWN)
    {
        if (!instance->mng.is_need_check ||
            ((msg->status0 == FI2C_MSG_COMPLETE_OK) && (msg->status1 == FI2C_MSG_SUCCESS)))
        {
            instance->abort_source = 0;
            result = FI2C_MSG_SUCCESS;
        }
        else
        {
            instance->abort_source = 1 << msg->status1;
            FI2C_MSG_ERROR("check result failed,status0 = %x\n, status1 =%x\n",
                           msg->status0, msg->status1);
        }
    }

    return result;
}

void FI2cMsgVirtDisable(FI2cMsgCtrl *const instance)
{
    FI2cMsgVirtSetModuleEn(instance, FI2C_MSG_ADAPTER_MODULE_OFF);

    FI2cMsgVirtCheckResult(instance);
}

void FI2cMsgVirtSetModuleEn(FI2cMsgCtrl *const instance, u8 data)
{
    FI2cMsgVirtSetCmd8(instance, FI2C_MSG_CMD_SET_MODULE_EN, data);
}

void FI2cMsgVirtSetSdaHold(FI2cMsgCtrl *const instance, u32 data)
{
    FI2cMsgVirtSetCmd32(instance, FI2C_MSG_CMD_SET_SDA_HOLD, data);
}

void FI2cMsgVirtSetIntTl(FI2cMsgCtrl *const instance, u8 tx_threshold, u8 rx_threshold)
{
    FI2cMsgInfo i2c_mng_msg;
    FI2cMsgFifoThreshold *ctrl = (FI2cMsgFifoThreshold *)&i2c_mng_msg.para[0];

    memset(&i2c_mng_msg, 0, sizeof(i2c_mng_msg));

    FI2cMsgCommonSetCmd(instance, &i2c_mng_msg, FMSG_CMD_SET, FI2C_MSG_CMD_SET_INT_TL);

    ctrl->tx_fifo_threshold = tx_threshold;
    ctrl->rx_fifo_threshold = rx_threshold;

    instance->total_shmem_len = FI2C_MSG_CMDDATA_SIZE;
    FI2cMsgVirtSendMsg(instance, &i2c_mng_msg, TRUE);
}

void FI2cMsgVirtSetDefaultCfg(FI2cMsgCtrl *const instance, FI2cMsgDefaultCfg *buf)
{
    FI2cMsgInfo i2c_mng_msg;
    memset(&i2c_mng_msg, 0, sizeof(i2c_mng_msg));

    FI2cMsgCommonSetCmd(instance, &i2c_mng_msg, FMSG_MSG_CMD_DEFAULT, FI2C_MSG_CMD_DEFAULT_RESUME);
    FI2cMsgMemcpy(&i2c_mng_msg.para[0], (char *)buf, sizeof(FI2cMsgDefaultCfg));
    instance->total_shmem_len = FI2C_MSG_CMDDATA_SIZE;
    FI2cMsgVirtSendMsg(instance, &i2c_mng_msg, TRUE);
}

void *FI2cMsgMemcpy(void *dest, const void *src, u32 len)
{
    while (len >= 8)
    {
        *(u64 *)dest = *(u64 *)src;
        dest += 8;
        src += 8;
        len -= 8;
    }
    while (len)
    {
        *(u8 *)dest = *(u8 *)src;
        dest++;
        src++;
        len--;
    }
    return dest;
}
