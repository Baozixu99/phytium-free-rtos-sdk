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
 * FilePath: fi2c_msg_master.c
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
#include "stdio.h"
#include "string.h"
#include "ftypes.h"
#include "fi2c_msg.h"
#include "fmsg_common.h"
#include "fi2c_msg_common.h"
#include "fi2c_msg_hw.h"
#include "fi2c_msg_master.h"
#include "fparameters.h"
#include "fdrivers_port.h"

/************************** Constant Definitions *****************************/
#define FI2C_MSG_CON_SPEED_MASK     (0x06)
#define FI2C_MSG_BYTE_ADDR_MASK     (0xFF)
#define FI2C_MSG_BYTE_ADDR_SHIFT8   (8)
#define FI2C_MSG_BYTE_ADDR_SHIFT16  (16)
#define FI2C_MSG_BYTE_ADDR_SHIFT24  (24)

#define FI2C_MSG_MNG_SIZE           (8)
#define FI2C_MSG_SHMEM_STORE_OFFSET (FI2C_MSG_MNG_SIZE + sizeof(FI2cMsgTransInfo))
#define FI2C_DEFAULT_TIMEOUT        (FI2C_MSG_DEFAULT_CLOCK_FREQENCY / 1000 * 35)

#define ETIMEOUT                    110
#define CH_DATA_FMT(byte0, byte1, byte2, byte3) \
    ((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FI2C_MSG_DEBUG_TAG "I2C-MSG-MASTER"
#define FI2C_MSG_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_MSG_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_MSG_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
static void FI2cMsgMasterTransferSingleFrame(FI2cMsgCtrl *instance)
{
    FI2cMsgInfo i2c_mng_msg;
    FI2cMsgTransInfo *i2c_msg_info;
    FI2cMsgInfo *shmem_msg = (FI2cMsgInfo *)instance->tx_shmem_addr;
    struct FI2cMsg *msgs = instance->msgs;

    int i, len = 0, remain_len;
    instance->total_cnt = FI2cMsgMasterTotalFrameCnt(instance);
    if (instance->total_cnt)
    {
        instance->total_cnt--;
    }
    for (i = 0; i < instance->msg_num; i++)
    {
        remain_len = msgs[i].len;
        instance->mng.opt_finish_len = 0;
        while (instance->mng.tx_cmd_cnt < instance->mng.tx_ring_cnt - 1)
        {
            instance->total_shmem_len = 0;
            memset(&i2c_mng_msg, 0, sizeof(i2c_mng_msg));
            i2c_mng_msg.seq = instance->total_cnt--;

            /*Set the Head.len*/
            len = remain_len;
            if (remain_len > FI2C_MSG_SIGLE_BUF_LEN)
            {
                len = FI2C_MSG_SIGLE_BUF_LEN;
            }
            i2c_mng_msg.len = len;
            /*Set other info of the Head*/
            i2c_mng_msg.cmd_type = FMSG_CMD_DATA;
            i2c_mng_msg.cmd_subid = FI2C_MSG_CMD_DATA_XFER;
            i2c_mng_msg.status0 = FI2C_MSG_COMPLETE_UNKNOWN;
            /*Store addr flags of struct i2c_msg*/
            i2c_msg_info = (FI2cMsgTransInfo *)&i2c_mng_msg.para[0];
            i2c_msg_info->addr = (u16)msgs[i].addr;
            i2c_msg_info->flags = (u16)msgs[i].flags;

            if (!instance->mng.tx_cmd_cnt)
            {
                i2c_msg_info->type = FI2C_MSG_TRANS_FRAME_START;
            }
            if ((i) && ((msgs[i - 1].flags & FI2C_M_RD) != (msgs[i].flags & FI2C_M_RD)))
            {
                if (remain_len == msgs[i].len)
                {
                    i2c_msg_info->type = FI2C_MSG_TRANS_FRAME_RESTART;
                }
            }
            if (((i + 1) >= instance->msg_num) && (remain_len == len))
            {
                instance->mng.is_last_frame = TRUE;
                i2c_msg_info->type |= FI2C_MSG_TRANS_FRAME_END;
            }
            instance->total_shmem_len = FI2C_MSG_SHMEM_STORE_OFFSET;

            if (!(msgs[i].flags & FI2C_M_RD))
            {
                FI2cMsgMemcpy(&i2c_mng_msg.para[sizeof(FI2cMsgTransInfo)],
                              &msgs[i].buf[instance->mng.opt_finish_len], len);

                instance->total_shmem_len += len;
            }
            /*Update to share memory*/
            FI2cMsgMemcpy(&shmem_msg[instance->mng.tx_cmd_cnt], &i2c_mng_msg, instance->total_shmem_len);
            instance->real_index[instance->mng.tx_cmd_cnt] = i;
            instance->mng.cur_index = i;
            instance->mng.opt_finish_len += len;
            instance->mng.tx_cmd_cnt++;

            remain_len -= len;
            if (!remain_len)
            {
                break;
            }
        }
    }

    FI2C_MSG_WRITE_REG32(instance->config.msg.regfile, FMSG_TX_TAIL, instance->mng.tx_cmd_cnt);

    FI2cMsgNotifySlave(instance, TRUE);
}

FError FI2cMsgMasterVirtXfer(FI2cMsgCtrl *const instance, struct FI2cMsg *msgs, u32 num)
{
    FError ret;
    instance->msgs = msgs;
    instance->msg_num = num;
    instance->cmd_err = 0;
    instance->abort_source = 0;
    instance->rx_buf_len = 0;
    instance->mng.tx_cmd_cnt = 0;
    instance->mng.cur_cmd_cnt = 0;
    instance->mng.is_last_frame = FALSE;

    FI2C_MSG_WRITE_REG32(instance->config.msg.regfile, FMSG_TX_HEAD, 0);
    FI2C_MSG_WRITE_REG32(instance->config.msg.regfile, FMSG_TX_TAIL, 0);

    FI2cMsgMasterTransferSingleFrame(instance);

    ret = FI2cMsgVirtCheckResult(instance);
    if (!ret)
    {
        ret = FI2C_MSG_SUCCESS;
    }
    else if (instance->abort_source != FI2C_MSG_SUCCESS)
    {
        if ((instance->abort_source & BIT(FI2C_MSG_TIMEOUT)) ||
            (instance->abort_source & BIT(FI2C_MSG_TRANS_PACKET_FAIL)))
        {
            FI2cMsgVirtSetModuleEn(instance, FI2C_MSG_ADAPTER_MODULE_RESET);
            FI2cMsgVirtCheckResult(instance);
            ret = FI2C_MSG_TX_ABRT;
        }
    }

    return ret;
}

static int FI2cMsgMasterUpdateMsgInfo(FI2cMsgCtrl *const instance)
{
    FI2cMsgInfo shmem_msg;
    FI2cMsgInfo *tx_shmem = (FI2cMsgInfo *)instance->tx_shmem_addr;
    FI2cMsgTransInfo *i2c_msg_info;
    struct FI2cMsg *msgs = instance->msgs;
    int remain_len = 0;

    if ((instance->mng.cur_index >= instance->msg_num) ||
        (msgs[instance->mng.cur_index].len < instance->mng.opt_finish_len))
    {
        return FI2C_MSG_PARA_ERR;
    }
    memset(&shmem_msg, 0, sizeof(FI2cMsgInfo));
    i2c_msg_info = (FI2cMsgTransInfo *)&shmem_msg.para[0];

    remain_len = msgs[instance->mng.cur_index].len - instance->mng.opt_finish_len;

    if (!remain_len)
    {
        if (instance->mng.cur_index + 1 >= instance->msg_num)
        {
            return FI2C_MSG_RUNNING;
        }

        instance->mng.cur_index++;

        if ((msgs[instance->mng.cur_index - 1].flags & FI2C_M_RD) !=
            (msgs[instance->mng.cur_index].flags & FI2C_M_RD))
        {
            i2c_msg_info->type = FI2C_MSG_TRANS_FRAME_RESTART;
        }
        instance->mng.opt_finish_len = 0;
        remain_len = msgs[instance->mng.cur_index].len;
    }
    shmem_msg.len = remain_len;
    if (remain_len > FI2C_MSG_SIGLE_BUF_LEN)
    {
        shmem_msg.len = FI2C_MSG_SIGLE_BUF_LEN;
    }
    else
    {
        if (instance->mng.cur_index + 1 >= instance->msg_num)
        {
            i2c_msg_info->type |= FI2C_MSG_TRANS_FRAME_END;
            instance->mng.is_last_frame = TRUE;
        }
    }

    shmem_msg.seq = instance->total_cnt--;
    shmem_msg.cmd_type = FMSG_CMD_DATA;
    shmem_msg.cmd_subid = FI2C_MSG_CMD_DATA_XFER;
    shmem_msg.status0 = FI2C_MSG_COMPLETE_UNKNOWN;

    i2c_msg_info->addr = msgs[instance->mng.cur_index].addr;
    i2c_msg_info->flags = msgs[instance->mng.cur_index].flags;

    instance->total_shmem_len = FI2C_MSG_SHMEM_STORE_OFFSET;

    if (!(msgs[instance->mng.cur_index].flags & FI2C_MSG_M_RD))
    {
        FI2cMsgMemcpy(&shmem_msg.para[sizeof(FI2cMsgTransInfo)],
                      &msgs[instance->mng.cur_index].buf[instance->mng.opt_finish_len],
                      shmem_msg.len);
        instance->total_shmem_len += shmem_msg.len;
    }

    instance->mng.opt_finish_len += shmem_msg.len;
    FI2cMsgMemcpy(&tx_shmem[instance->mng.tx_cmd_cnt % instance->mng.tx_ring_cnt],
                  &shmem_msg, instance->total_shmem_len);
    instance->real_index[instance->mng.tx_cmd_cnt % instance->mng.tx_ring_cnt] =
        instance->mng.cur_index;

    instance->mng.tx_cmd_cnt++;
    FI2C_MSG_WRITE_REG32(instance->config.msg.regfile, FMSG_TX_TAIL,
                         instance->mng.tx_cmd_cnt % instance->mng.tx_ring_cnt);
    return FI2C_MSG_RUNNING;
}

int FI2cMsgMasterHandle(FI2cMsgCtrl *const instance)
{
    FI2cMsgInfo *tx_shmem = (FI2cMsgInfo *)instance->tx_shmem_addr;
    FI2cMsgInfo shmem_msg;
    u32 tx_head;
    int ret = 0;
    if (instance->mng.cur_cmd_cnt >= instance->mng.tx_cmd_cnt)
    {
        return 0;
    }
    tx_head = instance->mng.cur_cmd_cnt % instance->mng.tx_ring_cnt;

    instance->mng.cur_cmd_cnt++;
    FI2cMsgMemcpy(&shmem_msg, &tx_shmem[tx_head], sizeof(FI2cMsgInfo));
    ret = FI2cMsgVirtCheckStatus(instance, &shmem_msg);
    if (ret)
    {
        FI2C_MSG_ERROR("FI2cMsgMasterHandle failed, ret = %d\n", ret);
        return FI2C_MSG_CHECK_STATUS_ERR;
    }

    FI2cMsgMasterVirtParseData(instance, &shmem_msg, tx_head);

    if (instance->mng.is_last_frame)
    {
        if (instance->mng.cur_index == instance->mng.tx_cmd_cnt)
        {
            FI2C_MSG_DEBUG("This buffer is_last_frame\n");
            return FI2C_MSG_SUCCESS;
        }
        return FI2C_MSG_RUNNING;
    }
    return FI2cMsgMasterUpdateMsgInfo(instance);
}

FError FI2cMsgMasterVirtParseData(FI2cMsgCtrl *const instance, FI2cMsgInfo *shmem_msg, int index)
{
    FError ret = FI2C_MSG_SUCCESS;
    int real_index;
    FI2cMsgTransInfo *i2c_msg_info;
    struct FI2cMsg *msgs = instance->msgs;
    i2c_msg_info = (FI2cMsgTransInfo *)&shmem_msg->para[0];
    real_index = instance->real_index[index];

    if (real_index >= instance->msg_num)
    {
        return FI2C_MSG_CNT_ERR;
    }
    if (i2c_msg_info->flags & FI2C_M_RD)
    {
        FI2cMsgMemcpy(&msgs[real_index].buf[instance->rx_buf_len],
                      &shmem_msg->para[sizeof(FI2cMsgTransInfo)], shmem_msg->len);
        instance->rx_buf_len += shmem_msg->len;
    }

    return ret;
}

FError FI2cMsgMasterVirtProbe(FI2cMsgCtrl *const instance)
{
    FError ret = FI2C_MSG_SUCCESS;

    ret = FI2cMsgCommonRegfileIntrDisable(instance);

    FI2cMsgCommonRegfileS2MIntrClear(instance, 0);

    if (ret != FI2C_MSG_SUCCESS)
    {
        FI2C_MSG_ERROR("FI2cMsgCommonRegfileIntrDisable failed, ret = %d\n", ret);
        return ret;
    }
    ret = FI2cMsgCommonRegfileIntrEnable(instance);
    if (ret != FI2C_MSG_SUCCESS)
    {
        FI2C_MSG_ERROR("FI2cMsgCommonRegfileIntrEnable failed, ret = %d\n", ret);
        return ret;
    }

    instance->intr_mask = FI2C_MSG_INTR_SMBUS_TIME_MASK;
    if (ret != FI2C_MSG_SUCCESS)
    {
        FI2C_MSG_ERROR("FI2cMsgMasterVirtInit failed, ret = %d\n", ret);
        return ret;
    }
    return ret;
}

int FI2cMsgMasterTotalFrameCnt(FI2cMsgCtrl *instance)
{
    int i, frame_cnt = 0, len = 0, remain_len = 0;
    struct FI2cMsg *set_msg = instance->msgs;

    for (i = 0; i < instance->msg_num; i++)
    {
        len = 0;
        remain_len = set_msg[i].len;
        while (len < remain_len)
        {
            if (remain_len > FI2C_MSG_SIGLE_BUF_LEN)
            {
                len += FI2C_MSG_SIGLE_BUF_LEN;
            }
            else
            {
                len = remain_len;
            }
            frame_cnt++;
        }
    }
    return frame_cnt;
}