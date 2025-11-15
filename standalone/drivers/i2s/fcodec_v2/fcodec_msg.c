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
 * FilePath: fcodec_msg.c
 * Created Date: 2025-05-16 16:48:35
 * Last Modified: 2025-06-12 15:26:25
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
#include "ftypes.h"
#include "fdrivers_port.h"
#include "fassert.h"
#include "fcodec_msg.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FCODEC_MSG_DEBUG_TAG "FCODEC-MSG"
#define FCODEC_MSG_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FCODEC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCODEC_MSG_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FCODEC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCODEC_MSG_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FCODEC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
static FError FI2sMsgCmd(FCodecMsgCtrl *instance_p, u32 cmd)
{
    FCodecMsgInfo *msg = (FCodecMsgInfo *)instance_p->config.msg.shmem;
    FError ret = FCODEC_MSG_SUCCESS;

    msg->reserved = 0;
    msg->seq = 0;
    msg->cmd_type = FMSG_CMD_SET;
    msg->cmd_subid = cmd;
    msg->complete = 0;
    ret = FCodecMsgSetCmd(instance_p);
    if (ret != FCODEC_MSG_SUCCESS)
    {
        FCODEC_MSG_ERROR("set cmd_subid 0x%x failed\n", cmd);
        ret = FCODEC_MSG_CMD_ERR;
        goto error;
    }
error:
    return ret;
}

/**
 * @name: FCodecDeInitialize
 * @msg: 完成Codec驱动实例去使能，清零实例数据
 * @return {*}
 * @param {FCodec} *instance_p
 */
void FCodecMsgDeInitialize(FCodecMsgCtrl *instance_p)
{
    FASSERT(instance_p);
    instance_p->is_ready = 0;

    FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_SHUTDOWN);
    FCodecMsgRemove(instance_p);
    memset(instance_p, 0, sizeof(*instance_p));
}

/**
 * @name: FCodecMsgCfgInitialize
 * @msg: 完成codec驱动实例的初始化，使之可以使用
 * @param {FCodecMsgCtrl} *instance_p codec驱动实例数据
 * @param {FCodecMsgConfig} *cofig_p codec驱动配置数据
 * @return success if initialization was successful
 *
 */
FError FCodecMsgCfgInitialize(FCodecMsgCtrl *instance_p, const FCodecMsgConfig *input_config_p)
{
    FASSERT(instance_p && input_config_p);

    FError ret = FCODEC_MSG_SUCCESS;
    FDevMsg *msg = &(instance_p->config.msg);

    /*
	 * If the device is started, disallow the initialize and return a Status
	 * indicating it is started.  This allows the user to de-initialize the device
	 * and reinitialize, but prevents a user from inadvertently
	 * initializing.
	 */
    if (FT_COMPONENT_IS_READY == instance_p->is_ready)
    {
        FCODEC_MSG_ERROR("Device is already initialized!!!");
        return FCODEC_MSG_NOT_INIT;
    }
    /*
	 * Set default values and configuration data, including setting the
	 * callback handlers to stubs  so the system will not crash should the
	 * application not assign its own callbacks.
	 */
    FCodecMsgDeInitialize(instance_p);
    if (&instance_p->config != input_config_p)
    {
        instance_p->config = *input_config_p;
    }
    instance_p->is_ready = FT_COMPONENT_IS_READY;

    instance_p->timeout = 5000;

    return ret;
}

static void FI2sMsgShowStatus(u8 status)
{
    switch (status)
    {
        case 0:
            FCODEC_MSG_ERROR("success\n");
            break;
        case 2:
            FCODEC_MSG_ERROR("device busy\n");
            break;
        case 3:
            FCODEC_MSG_ERROR("read/write error\n");
            break;
        case 4:
            FCODEC_MSG_ERROR("no device\n");
            break;
        default:
            FCODEC_MSG_ERROR("unknown error: %d\n", status);
            break;
    }
}

FError FCodecMsgSetCmd(FCodecMsgCtrl *instance_p)
{
    FCodecMsgInfo *ans_msg;
    u32 timeout = instance_p->timeout;
    FError ret = FCODEC_MSG_SUCCESS;

    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.regfile, FMSG_M2S_INT_STATE, SEND_INTR);

    ans_msg = (FCodecMsgInfo *)instance_p->config.msg.shmem;

    while (timeout && (ans_msg->complete == FCODEC_COMPLETE_NOT_READY ||
                       ans_msg->complete == FCODEC_COMPLETE_GOING))
    {
        FDriverUdelay(200);
        timeout--;
    }
    if (timeout == 0)
    {
        FCODEC_MSG_ERROR("failed to receive msg, timeout\n");
        ret = FCODEC_MSG_TIME_OUT;
    }
    else if (ans_msg->complete >= FCODEC_COMPLETE_GENERIC_ERROR)
    {
        FCODEC_MSG_ERROR("receive msg; generic_error, error code:%d\n", ans_msg->complete);
        ret = FCODEC_MSG_TIME_OUT;
    }
    else if (ans_msg->complete == FCODEC_COMPLETE_SUCCESS)
    {
        FCODEC_MSG_DEBUG("receive msg successfully\n");
    }

    if (ans_msg->complete != FCODEC_COMPLETE_SUCCESS)
    {
        FI2sMsgShowStatus(ans_msg->status);
    }
    return ret;
}

static FError FCodecMsgPmCmd(FCodecMsgCtrl *instance_p, u32 cmd)
{
    FCodecMsgInfo *msg = (FCodecMsgInfo *)instance_p->config.msg.shmem;
    FError ret = FCODEC_MSG_SUCCESS;
    u16 total_regs_len;
    u8 *regs;

    memset(msg, 0, sizeof(FCodecMsgInfo));

    msg->reserved = 0;
    msg->seq = 0;
    msg->cmd_type = FMSG_CMD_SET;
    msg->cmd_subid = cmd;
    msg->complete = 0;
    msg->cmd_para.codec_reg.cnt = 0;
    if (cmd == FCODEC_MSG_CMD_SET_RESUME)
    {
        memcpy(msg->cmd_para.codec_reg.regs, instance_p->regs, REG_SH_LEN);
    }
    ret = FCodecMsgSetCmd(instance_p);
    if (ret != FCODEC_MSG_SUCCESS)
    {
        FCODEC_MSG_ERROR("set cmd_subid 0x%x failed\n", cmd);
        ret = FCODEC_MSG_CMD_ERR;
        goto error;
    }
    total_regs_len = msg->cmd_para.codec_reg.total_regs_len;

    if (cmd == FCODEC_MSG_CMD_SET_SUSPEND)
    {
        regs = malloc(total_regs_len);
        instance_p->regs = regs;
        while (total_regs_len > REG_SH_LEN * msg->cmd_para.codec_reg.cnt)
        {
            memcpy(regs, msg->cmd_para.codec_reg.regs, REG_SH_LEN);
            regs += REG_SH_LEN;
            msg->complete = 0;
            ret = FCodecMsgSetCmd(instance_p);
            if (ret != FCODEC_MSG_SUCCESS)
            {
                FCODEC_MSG_ERROR("set cmd_subid 0x%x failed\n", cmd);
                ret = FCODEC_MSG_CMD_ERR;
                goto error;
            }
        }
        memcpy(regs, msg->cmd_para.codec_reg.regs,
               total_regs_len - REG_SH_LEN * (msg->cmd_para.codec_reg.cnt - 1));
    }
    else if (cmd == FCODEC_MSG_CMD_SET_RESUME)
    {
        regs = instance_p->regs;
        while (total_regs_len > REG_SH_LEN * msg->cmd_para.codec_reg.cnt)
        {
            regs += REG_SH_LEN;
            memcpy(msg->cmd_para.codec_reg.regs, regs, REG_SH_LEN);
            msg->complete = 0;
            ret = FCodecMsgSetCmd(instance_p);
            if (ret < 0)
            {
                FCODEC_MSG_ERROR("set cmd_subid 0x%x failed\n", cmd);
                ret = FCODEC_MSG_CMD_ERR;
                goto error;
            }
        }
        free(instance_p->regs);
    }
error:
    return ret;
}

FError FCodecMsgProbe(FCodecMsgCtrl *instance_p)
{
    return FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_PROBE);
}

FError FCodecMsgRemove(FCodecMsgCtrl *instance_p)
{
    FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_REMOVE);
}

FError FCodecMsgSuspend(FCodecMsgCtrl *instance_p)
{
    return FCodecMsgPmCmd(instance_p, FCODEC_MSG_CMD_SET_SUSPEND);
}

FError FCodecMsgResume(FCodecMsgCtrl *instance_p)
{
    return FCodecMsgPmCmd(instance_p, FCODEC_MSG_CMD_SET_RESUME);
}

FError FCodecMsgSetBias(FCodecMsgCtrl *instance_p, FCodecMsgBias level)
{
    FError ret = FCODEC_MSG_SUCCESS;

    FCodecMsgInfo *msg = (FCodecMsgInfo *)instance_p->config.msg.shmem;

    memset(msg, 0, sizeof(FCodecMsgInfo));
    msg->cmd_para.para[0] = instance_p->channels / 2;

    switch (level)
    {
        case FCODEC_MSG_BIAS_ON:
            ret = FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_BIAS_ON);
            break;

        case FCODEC_MSG_BIAS_PREPARE:
            ret = FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_BIAS_PREPARE);
            break;

        case FCODEC_MSG_BIAS_STANDBY:
            ret = FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_BIAS_STANDBY);
        case FCODEC_MSG_BIAS_OFF:
            ret = FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_BIAS_OFF);
            break;
    }

    return ret;
}

FError FCodecMsgMute(FCodecMsgCtrl *instance_p, int mute, int direction)
{
    FError ret = FCODEC_MSG_SUCCESS;

    FCodecMsgInfo *msg = (FCodecMsgInfo *)instance_p->config.msg.shmem;

    memset(msg, 0, sizeof(FCodecMsgInfo));

    msg->cmd_para.para[0] = (uint8_t)direction;
    if (mute)
    {
        ret = FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_MUTE);
    }
    else
    {
        ret = FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_UNMUTE);
    }

    return ret;
}

FError FCodecMsgStartup(FCodecMsgCtrl *instance_p, u32 mode)
{
    FError ret = FCODEC_MSG_SUCCESS;

    FCodecMsgInfo *msg = (FCodecMsgInfo *)instance_p->config.msg.shmem;

    memset(msg, 0, sizeof(FCodecMsgInfo));

    if (mode == AUDIO_PCM_STREAM_PLAYBACK)
    {
        ret = FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_STARTUP);
    }
    else
    {
        ret = FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_STARTUP_RC);
    }

    return ret;
}

FError FCodecMsgShutDown(FCodecMsgCtrl *instance_p, u32 mode)
{
    FError ret = FCODEC_MSG_SUCCESS;

    FCodecMsgInfo *msg = (FCodecMsgInfo *)instance_p->config.msg.shmem;

    memset(msg, 0, sizeof(FCodecMsgInfo));

    if (mode == AUDIO_PCM_STREAM_PLAYBACK)
    {
        ret = FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_SHUTDOWN);
    }
    else
    {
        ret = FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_SHUTDOWN_RC);
    }

    return ret;
}

FError FCodecMsgSetFmt(FCodecMsgCtrl *instance_p)
{
    FError ret = FCODEC_MSG_SUCCESS;

    FCodecMsgInfo *msg = (FCodecMsgInfo *)instance_p->config.msg.shmem;

    memset(msg, 0, sizeof(FCodecMsgInfo));
    ret = FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_DAI_FMT);
    return ret;
}

void FCodecSetPara(FCodecMsgCtrl *instance_p)
{
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.regfile, FCODEC_HW_PARAM, 3);
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.regfile, FCODEC_HW_PARAM_RC, 3);
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.regfile, FCODEC_PLAYBACKE_VOL, 0xc0);
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.regfile, FCODEC_PLAYBACKE_OUT1_VOL, 0x24);
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.regfile, FCODEC_PLAYBACKE_OUT2_VOL, 0x24);
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.regfile, FCODEC_CAPTURE_VOL, 0xc0);
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.regfile, FCODEC_CAPTURE_IN1_VOL, 4); /*8 max vol*/
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.regfile, FCODEC_ADC_ENABLE, 0x1);
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.regfile, FCODEC_INMUX_ENABLE, 0x1);
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.regfile, FCODEC_DAC_ENABLE, 0x1);
}

void FCodecSelect(FCodecMsgCtrl *instance_p)
{
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.shmem, 0x00, 0x01000000);
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.shmem, 0x04, 0x00000004);
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.shmem, 0x08, 0x02100103);
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.shmem, 0x0c, 0x00000000);
    FCODEC_MSG_WRITE_REG32(instance_p->config.msg.regfile, 0x24, 0x10);
}

void FCodecStop(FCodecMsgCtrl *instance_p)
{
    FCodecMsgInfo *msg = (FCodecMsgInfo *)instance_p->config.msg.shmem;

    memset(msg, 0, sizeof(FCodecMsgInfo));
    FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_SHUTDOWN);
    FI2sMsgCmd(instance_p, FCODEC_MSG_CMD_SET_SHUTDOWN_RC);
    FCodecMsgDeInitialize(instance_p);
}