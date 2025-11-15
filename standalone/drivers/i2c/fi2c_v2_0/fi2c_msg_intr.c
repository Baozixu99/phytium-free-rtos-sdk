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
 * FilePath: fi2c_msg_intr.c
 * Date: 2024-10-1 14:53:42
 * LastEditTime: 2024-10-10 08:36:14s
 * Description:  This file is for I2C msg intr configuration
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangzq     2024/10/1  first commit
 */

/***************************** Include Files *********************************/
#include <string.h>
#include "ftypes.h"
#include "fassert.h"
#include "fi2c_msg.h"
#include "fi2c_msg_common.h"
#include "fmsg_common.h"
#include "fi2c_msg_hw.h"
#include "fi2c_msg_master.h"
#include "fparameters.h"
#include "fdrivers_port.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FI2C_MSG_DEBUG_TAG "I2C-MSG_INTR"
#define FI2C_MSG_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_MSG_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_MSG_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_MSG_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
static void FI2cMsgSlaveEvent(FI2cMsgCtrl *instance_p, u8 evt, u8 *para)
{
    if (instance_p->slave_evt_handlers[evt])
    {
        instance_p->slave_evt_handlers[evt](instance_p, para);
    }
}
FError FI2cMsgVirtSlaveEventProcess(FI2cMsgCtrl *instance_p, FI2cMsgInfo *rx_msg,
                                    u32 rx_head, u32 rx_tail)
{
    FError ret = FI2C_MSG_SUCCESS;
    u8 buf[32] = {1};
    if ((rx_head >= FI2C_IO_SLAVE_EVENT_MAX_CNT) || (rx_tail >= FI2C_IO_SLAVE_EVENT_MAX_CNT))
    {
        FI2C_MSG_ERROR("head or tail is out of range, rx_head = %d, rx_tail = %d\n", rx_head, rx_tail);
        return FI2C_MSG_BUF_ERR;
    }
    while (rx_head != rx_tail)
    {
        if (rx_head % FI2C_IO_SLAVE_RX_INFO_SIZE)
        {
            rx_head++;
        }
        FI2cMsgSlaveEvent(instance_p, rx_msg->para[rx_head], &(rx_msg->para[rx_head + 1]));
        if ((rx_msg->para[rx_head] == FI2C_MSG_EVT_READ_REQUESTED) ||
            (rx_msg->para[rx_head] == FI2C_MSG_EVT_READ_PROCESSED))
        {
            buf[1] = rx_msg->para[rx_head + 1];
            FI2cMsgVirtSetCmdArray(instance_p, FI2C_MSG_CMD_DATA_SLAVE, buf, 2);
        }
        rx_head += FI2C_IO_SLAVE_RX_INFO_SIZE;
        if (rx_head >= FI2C_IO_SLAVE_EVENT_MAX_CNT)
        {
            rx_head = 0;
        }
    }
    FI2C_MSG_WRITE_REG32(instance_p->config.msg.regfile, FMSG_RX_HEAD, rx_head);
    return ret;
}

static inline FError FI2cMsgCallEvtHandler(FI2cMsgCtrl *const instance_p,
                                           FI2cMsgInfo *rx_msg, u32 head, u32 tail)
{
    FError ret = FI2C_MSG_SUCCESS;
    ret = FI2cMsgVirtSlaveEventProcess(instance_p, rx_msg, head, tail);
    return ret;
}

FError FI2cMsgCommonRegfileIntrDisable(FI2cMsgCtrl *const instance)
{
    FError ret = FI2C_MSG_SUCCESS;
    FDevMsg *msg = &(instance->config.msg);

    FI2C_MSG_WRITE_REG32(msg->regfile, FMSG_S2M_INT_MASK, FI2C_MSG_REGFILE_DISABLE_INTR_VAL);

    return ret;
}

FError FI2cMsgCommonRegfileIntrEnable(FI2cMsgCtrl *const instance)
{
    FError ret = FI2C_MSG_SUCCESS;
    FDevMsg *msg = &(instance->config.msg);

    FI2C_MSG_WRITE_REG32(msg->regfile, FMSG_S2M_INT_MASK, FI2C_MSG_REGFILE_ENABLE_INTR_VAL);
    return ret;
}

void FI2cMsgCommonRegfileS2MIntrClear(FI2cMsgCtrl *const instance, u32 state)
{
    /*for desktop type*/
    FI2C_MSG_WRITE_REG32(instance->config.msg.regfile, FMSG_S2M_INT_STATE, state & ~FI2C_MSG_S2M_INTR_4);
    /*for service type*/
    FI2C_MSG_WRITE_REG32(instance->config.msg.regfile, FMSG_S2M_INT_CLEAN, state | FI2C_MSG_S2M_INTR_4);
    return;
}

int FI2cMsgSlaveRegfileIsr(int irq, void *dev_id)
{
    u32 stat;
    FI2cMsgCtrl *dev = (FI2cMsgCtrl *)dev_id;
    stat = FI2C_MSG_READ_REG32(dev->config.msg.regfile, FMSG_S2M_INT_STATE);
    if (!(stat & FI2C_MSG_S2M_INTR_4))
    {
        return 0;
    }
    FI2cMsgCommonRegfileS2MIntrClear(dev, stat);
    if (dev->complete_flag)
    {
        dev->complete_flag = FALSE;
    }
    else
    {
        FI2cMsgSlaveIsrHandler(dev);
    }
    return 0;
}

void FI2cMsgMasterRegfileIsr(int irq, void *dev_id)
{
    FI2cMsgCtrl *dev = (FI2cMsgCtrl *)dev_id;
    FI2cMsgInfo *rx_msg = (FI2cMsgInfo *)dev->rx_shmem_addr;
    u32 stat, head, tail;
    u32 ret;
    stat = FI2C_MSG_READ_REG32(dev->config.msg.regfile, FMSG_S2M_INT_STATE);
    if (!(stat & FI2C_MSG_S2M_INTR_4))
    {
        FI2C_MSG_INFO("No FI2C_MSG_S2M_INTR_4, stat = 0x%x\n", stat);
        return;
    }
    FI2cMsgCommonRegfileS2MIntrClear(dev, stat);

    head = FI2C_MSG_READ_REG32(dev->config.msg.regfile, FMSG_TX_HEAD) % dev->mng.tx_ring_cnt;
    tail = dev->mng.cur_cmd_cnt % dev->mng.tx_ring_cnt;
    do
    {
        tail++;
        tail %= dev->mng.tx_ring_cnt;
        if (dev->complete_flag)
        {
            if (rx_msg->cmd_type == FMSG_CMD_REPORT)
            {
                goto done;
            }

            ret = FI2cMsgMasterHandle(dev);
            if (ret == FI2C_MSG_RUNNING)
            {
                continue;
            }

            dev->complete_flag = FALSE;
            dev->mng.cur_cmd_cnt = 0;
            FI2C_MSG_WRITE_REG32(dev->config.msg.regfile, FMSG_TX_HEAD, 0);
            FI2C_MSG_WRITE_REG32(dev->config.msg.regfile, FMSG_TX_TAIL, 0);
            return;
        }

    } while (tail != head);

done:
    FI2cMsgMasterIsrHandler(dev);
    return;
}

static int FI2cMsgReportCmdHandler(FI2cMsgCtrl *const instance, FI2cMsgInfo *rx_msg, u32 head, u32 tail)
{
    FError ret = FI2C_MSG_SUCCESS;
    return ret;
}

static void FI2cMsgVirtSetComplete(FI2cMsgCtrl *instance)
{
    FI2cMsgInfo *set_msg = (FI2cMsgInfo *)instance->rx_shmem_addr;
    FI2cMsgInfo tmp_msg;
    FI2cMsgMemcpy(&tmp_msg, &set_msg[0], sizeof(FI2cMsgInfo));
    tmp_msg.status0 = FI2C_MSG_COMPLETE_OK;
    tmp_msg.cmd_type = 0xff;
    tmp_msg.cmd_subid = 0xff;
    FI2cMsgMemcpy(&set_msg[0], &tmp_msg, sizeof(FI2cMsgInfo));
}

void FI2cMsgSlaveIsrHandler(FI2cMsgCtrl *const instance)
{
    FI2cMsgInfo *set_msg = (FI2cMsgInfo *)instance->rx_shmem_addr;
    FI2cMsgInfo msg_buf;

    u32 head, tail;
    tail = FI2C_MSG_READ_REG32(instance->config.msg.regfile, FMSG_RX_TAIL);
    head = FI2C_MSG_READ_REG32(instance->config.msg.regfile, FMSG_RX_HEAD);

    FI2cMsgMemcpy(&msg_buf, &set_msg[0], sizeof(msg_buf));
    if (msg_buf.cmd_type == FMSG_CMD_REPORT)
    {
        FI2cMsgReportCmdHandler(instance, &msg_buf, head, tail);
    }
}

void FI2cMsgMasterIsrHandler(FI2cMsgCtrl *const instance)
{
    FI2cMsgInfo *set_msg = (FI2cMsgInfo *)instance->rx_shmem_addr;
    FI2cMsgInfo msg_buf;
    FI2cMsgMemcpy(&msg_buf, &set_msg[0], sizeof(FI2cMsgInfo));
    FI2cMsgVirtSetComplete(instance);
    if (msg_buf.cmd_type == FMSG_CMD_REPORT)
    {
        FI2cMsgReportCmdHandler(instance, &msg_buf, 0, 0);
    }
}

void FI2cMsgTrigSlaveIntr(FI2cMsgCtrl *const instance)
{
    FDevMsg *msg = &(instance->config.msg);
    FI2C_MSG_WRITE_REG32(msg->regfile, FMSG_M2S_INT_STATE, FI2C_MSG_REGFILE_M2S_SET_INTR_VAL);
}

void FI2cMsgSetInterrupt(FI2cMsgCtrl *const instance, u32 is_enable, u32 intr_mask)
{
    FI2cMsgInfo i2c_mng_msg;
    u32 *data = (u32 *)i2c_mng_msg.para;

    memset(&i2c_mng_msg, 0, sizeof(i2c_mng_msg));

    FI2cMsgCommonSetCmd(instance, &i2c_mng_msg, FMSG_CMD_SET, FI2C_MSG_CMD_SET_INTERRUPT);

    data[0] = is_enable;
    data[1] = intr_mask;
    instance->total_shmem_len = FI2C_MSG_CMDDATA_SIZE;
    FI2cMsgVirtSendMsg(instance, &i2c_mng_msg, TRUE);
}