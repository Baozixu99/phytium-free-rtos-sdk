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
 * FilePath: fi2s_msg.c
 * Created Date: 2025-05-15 10:29:48
 * Last Modified: 2025-06-11 17:39:45
 * Description:  This file is for
 *
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

/***************************** Include Files *********************************/
#include <string.h>
#include <stdlib.h>
#include "fio.h"
#include "ferror_code.h"
#include "fparameters.h"
#include "ftypes.h"
#include "fdrivers_port.h"
#include "fassert.h"
#include "fi2s_msg.h"
#include "fi2s_msg_hw.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FI2S_MSG_DEBUG_TAG "I2S-MSG"
#define FI2S_MSG_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FI2S_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2S_MSG_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FI2S_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2S_MSG_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FI2S_MSG_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
/**
 * @name: FI2sDeInitialize
 * @msg: 完成I2S驱动实例去使能，清零实例数据
 * @return {*}
 */
void FI2sMsgDeInitialize(FI2sMsgCtrl *instance_p)
{
    FASSERT(instance_p);
    instance_p->is_ready = 0;

    memset(instance_p, 0, sizeof(*instance_p));
}

/**
 * @name: FI2sMsgCfgInitialize
 * @msg: 完成I2S驱动实例的初始化，使之可以使用
 * @param {FI2sMsgCtrl} *instance_p I2S驱动实例数据
 * @param {FI2sMsgConfig} *cofig_p I2S驱动配置数据
 * @return SUCCESS if initialization was successful
 *
 */
FError FI2sMsgCfgInitialize(FI2sMsgCtrl *instance_p, const FI2sMsgConfig *input_config_p)
{
    FASSERT(instance_p && input_config_p);

    FError ret = FI2S_MSG_SUCCESS;
    FDevMsg *msg = &(instance_p->config.msg);

    /*
     * If the device is started, disallow the initialize and return a Status
     * indicating it is started.  This allows the user to de-initialize the device
     * and reinitialize, but prevents a user from inadvertently
     * initializing.
     */
    if (FT_COMPONENT_IS_READY == instance_p->is_ready)
    {
        FI2S_MSG_ERROR("Device is already initialized!!!");
        return FI2S_MSG_NOT_INIT;
    }

    /*
     * Set default values and configuration data, including setting the
     * callback handlers to stubs  so the system will not crash should the
     * application not assign its own callbacks.
     */
    FI2sMsgDeInitialize(instance_p);
    if (&instance_p->config != input_config_p)
    {
        instance_p->config = *input_config_p;
    }
    instance_p->is_ready = FT_COMPONENT_IS_READY;
    return ret;
}

FError FI2sMsgSetCmd(FI2sMsgCtrl *instance_p, FI2sMsgInfo *set_msg)
{
    FI2sMsgInfo *ans_msg;
    u32 time_out = 40, ret = 0;

    memcpy((FI2sMsgInfo *)instance_p->config.msg.shmem, set_msg, sizeof(FI2sMsgInfo));

    FI2S_MSG_WRITE_REG32(instance_p->config.msg.regfile, FMSG_M2S_INT_STATE, SEND_INTR);
    ans_msg = (FI2sMsgInfo *)instance_p->config.msg.shmem;
    while ((ans_msg->status == FI2S_COMPLETE_NONE || ans_msg->status == FI2S_COMPLETE_GOING) && time_out)
    {
        FDriverUdelay(1000);
        time_out--;
    }
    if (time_out == 0)
    {
        FI2S_MSG_ERROR("wait cmd reply timeout");
        ret = FI2S_MSG_TIME_OUT;
    }
    if (ans_msg->status == FI2S_COMPLETE_ERROR)
    {
        FI2S_MSG_ERROR("handle cmd err");
        ret = FI2S_MSG_HANDLE_ERR;
    }
    if (ans_msg->status == FI2S_COMPLETE_ID_NOT_SUPPORTED)
    {
        FI2S_MSG_ERROR("cmd err");
        ret = FI2S_MSG_CMD_ERR;
    }
    if (ans_msg->status == FI2S_COMPLETE_INVALID_PARAMETERS)
    {
        FI2S_MSG_ERROR("para err");
        ret = FI2S_MSG_PARA_ERR;
    }
    return ret;
}

FError FI2sMsgHwSetParams(FI2sMsgCtrl *instance_p, u8 mode, u32 sample_rate, u8 chan, u8 enable)
{
    FI2sMsgInfo *msg = (FI2sMsgInfo *)instance_p->config.msg.shmem;
    FError ret = FI2S_MSG_SUCCESS;
    FI2sMsgModeData data;
    memset(&data, 0, sizeof(FI2sMsgModeData));
    memset(msg, 0, sizeof(FI2sMsgInfo));
    switch (instance_p->data_width)
    {
        case AUDIO_PCM_STREAM_WORD_LENGTH_16:
            data.data_width = 16;
            break;
        case AUDIO_PCM_STREAM_WORD_LENGTH_24:
            data.data_width = 24;
            break;
        case AUDIO_PCM_STREAM_WORD_LENGTH_32:
            data.data_width = 32;
            break;
        default:
            FI2S_MSG_ERROR("i2s unsupported this dai word length");
            free(msg); // 释放内存
            ret = FI2S_MSG_PARA_ERR;
            break;
    }
    data.direction = mode;
    data.sample_rate = sample_rate;
    data.chan_nr = chan;
    data.enable = enable;
    data.clk_base = FI2S_CLK_FREQ_HZ;

    msg->seq = FI2S_LSD_ID;
    msg->cmd_type = FMSG_CMD_SET;
    msg->status = 0;
    msg->cmd_subid = FI2S_MSG_CMD_SET_MODE;
    memcpy(msg->para, &data, sizeof(FI2sMsgModeData));

    ret = FI2sMsgSetCmd(instance_p, msg);
    if (ret != FI2S_MSG_SUCCESS)
    {
        FI2S_MSG_ERROR("set mode failed");
    }
    return ret;
}

FError FI2sMsgStart(FI2sMsgCtrl *instance_p, u8 mode, boolean enable)
{
    FI2sMsgInfo *msg = (FI2sMsgInfo *)instance_p->config.msg.shmem;
    FI2sMsgTriggerInfo data;
    FError ret = FI2S_MSG_SUCCESS;

    memset(&data, 0, sizeof(FI2sMsgTriggerInfo));
    memset(msg, 0, sizeof(FI2sMsgInfo));

    data.direction = mode;
    data.start = enable;
    msg->seq = FI2S_LSD_ID;
    msg->cmd_type = FMSG_CMD_SET;
    msg->cmd_subid = FI2S_MSG_CMD_SET_TRIGGER;
    msg->status = 0;

    memcpy(msg->para, &data, sizeof(FI2sMsgTriggerInfo));
    ret = FI2sMsgSetCmd(instance_p, msg);
    if (ret != FI2S_MSG_SUCCESS)
    {
        FI2S_MSG_ERROR("start or stop i2s failed");
    }
    return FI2S_MSG_SUCCESS;
}
