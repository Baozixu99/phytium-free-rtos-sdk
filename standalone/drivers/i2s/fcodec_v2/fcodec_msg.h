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
 * FilePath: fcodec_msg.h
 * Created Date: 2025-05-16 16:48:43
 * Last Modified: 2025-06-11 10:51:22
 * Description:  This file is for
 *
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

#ifndef FCODEC_MSG_H
#define FCODEC_MSG_H

/***************************** Include Files *********************************/
#include "ftypes.h"
#include "fmsg_common.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
#define FCODEC_MSG_READ_REG32(addr, reg_offset) FtIn32((addr) + (u32)(reg_offset))
#define FCODEC_MSG_WRITE_REG32(addr, reg_offset, reg_value) \
    FtOut32((addr) + (u32)reg_offset, (u32)reg_value)

#define FCODEC_MSG_SUCCESS        FT_SUCCESS

#define FCODEC_MSG_DATA_LEN       56

#define FCODEC_MSG_NOT_INIT       FT_MAKE_ERRCODE(ErrModBsp, ErrBspCodecMsg, 1)
#define FCODEC_MSG_PARA_ERR       FT_MAKE_ERRCODE(ErrModBsp, ErrBspCodecMsg, 2)
#define FCODEC_MSG_CMD_ERR        FT_MAKE_ERRCODE(ErrModBsp, ErrBspCodecMsg, 3)
#define FCODEC_MSG_HANDLE_ERR     FT_MAKE_ERRCODE(ErrModBsp, ErrBspCodecMsg, 4)
#define FCODEC_MSG_TIME_OUT       FT_MAKE_ERRCODE(ErrModBsp, ErrBspCodecMsg, 5)

#define FCODEC_PLAYBACKE_VOL      0x500 /*set dac vol*/
#define FCODEC_PLAYBACKE_OUT1_VOL 0x504 /*set output 1 vol*/
#define FCODEC_PLAYBACKE_OUT2_VOL 0x508 /*set output 2 vol*/
#define FCODEC_CAPTURE_VOL        0x50c /*ADC vol*/
#define FCODEC_CAPTURE_IN1_VOL    0x510 /* capture input 1 vol*/
#define FCODEC_HW_PARAM           0x514 /*playback para*/
#define FCODEC_HW_PARAM_RC        0x518 /*capture para*/
#define FCODEC_INMUX_ENABLE       0x51c /*input enable*/
#define FCODEC_INMUX_SEL          0x520 /*input select*/
#define FCODEC_ADC_ENABLE         0x524 /*ADC enable*/
#define FCODEC_DAC_ENABLE         0x528 /*DAC enable*/
#define FCODEC_INT_STATUS         0x560
#define PIPE_NUM                  11

#define REG_MAX                   0x52c
#define REG_SH_LEN                52
#define SEND_INTR                 (1 << 4)

/****************register end *****************/
#define FCODEC_LSD_ID             0x701

typedef enum
{
    AUDIO_PCM_STREAM_CAPTURE = 0, /* work mode*/
    AUDIO_PCM_STREAM_PLAYBACK,
    AUDIO_PCM_STREAM_TYPE
} FCodecMsgFormat;

typedef enum
{
    FCODEC_MSG_BIAS_OFF = 0,
    FCODEC_MSG_BIAS_STANDBY = 1,
    FCODEC_MSG_BIAS_PREPARE = 2,
    FCODEC_MSG_BIAS_ON = 3,
} FCodecMsgBias;

typedef enum
{
    FCODEC_MSG_CMD_SET_PROBE = 0,
    FCODEC_MSG_CMD_SET_REMOVE,
    FCODEC_MSG_CMD_SET_SUSPEND,
    FCODEC_MSG_CMD_SET_RESUME,
    FCODEC_MSG_CMD_SET_STARTUP,
    FCODEC_MSG_CMD_SET_STARTUP_RC,
    FCODEC_MSG_CMD_SET_MUTE,
    FCODEC_MSG_CMD_SET_UNMUTE,
    FCODEC_MSG_CMD_SET_DAI_FMT,
    FCODEC_MSG_CMD_SET_BIAS_ON,
    FCODEC_MSG_CMD_SET_BIAS_OFF,
    FCODEC_MSG_CMD_SET_BIAS_PREPARE,
    FCODEC_MSG_CMD_SET_BIAS_STANDBY,
    FCODEC_MSG_CMD_SET_SHUTDOWN,
    FCODEC_MSG_CMD_SET_SHUTDOWN_RC,
} FCodecMsgSetSubid;

typedef enum
{
    FCODEC_COMPLETE_NOT_READY = 0,
    FCODEC_COMPLETE_SUCCESS,
    FCODEC_COMPLETE_GOING,
    FCODEC_COMPLETE_GENERIC_ERROR = 0x10,
    FCODEC_COMPLETE_TYPE_NOT_SUPPORTED,
    FCODEC_COMPLETE_CMD_NOT_SUPPORTED,
    FCODEC_COMPLETE_INVALID_PARAMETERS,
} FCodecMsgComplete;

typedef struct
{
    u16 total_regs_len;
    u8 one_reg_len;
    u8 cnt;
    u8 regs[52];
} FCodecMsgReg;

typedef struct
{
    u32 instance_id;
    FDevMsg msg;
} FCodecMsgConfig;

typedef struct
{
    u8 reserved;
    u8 seq;      /*由发起方使用或维护，对端不改变该值，默认为0*/
    u8 cmd_type; /*功能性命令字段，可以用极少位表示，当前保留为8bit*/
    u8 cmd_subid; /*子命令id，与命令实现有关，最多为256个，搭配cmd_type补充*/
    u16 len;     /*在交互数据时，表示payload的长度，交互命令时为0*/
    u8 status;   /*协议命令状态，表示命令执行相应*/
    u8 complete; /*协议完成状态*/
    union
    {
        u8 para[FCODEC_MSG_DATA_LEN];
        FCodecMsgReg codec_reg;
    } cmd_para;
} FCodecMsgInfo;

typedef struct
{
    FCodecMsgConfig config;
    u8 channels;
    u32 timeout;
    u8 *regs;
    u32 is_ready;
} FCodecMsgCtrl;

const FCodecMsgConfig *FCodecMsgLookupConfig(u32 instance_id);

FError FCodecMsgShutDown(FCodecMsgCtrl *instance_p, u32 mode);

FError FCodecMsgStartup(FCodecMsgCtrl *instance_p, u32 mode);

FError FCodecMsgMute(FCodecMsgCtrl *instance_p, int mute, int direction);

FError FCodecMsgSetBias(FCodecMsgCtrl *instance_p, FCodecMsgBias level);

FError FCodecMsgResume(FCodecMsgCtrl *instance_p);

FError FCodecMsgSuspend(FCodecMsgCtrl *instance_p);

FError FCodecMsgRemove(FCodecMsgCtrl *instance_p);

FError FCodecMsgProbe(FCodecMsgCtrl *instance_p);

FError FCodecMsgSetFmt(FCodecMsgCtrl *instance_p);

FError FCodecMsgSetCmd(FCodecMsgCtrl *instance_p);

void FCodecSetPara(FCodecMsgCtrl *instance_p);

FError FCodecMsgCfgInitialize(FCodecMsgCtrl *instance_p, const FCodecMsgConfig *input_config_p);

void FCodecMsgDeInitialize(FCodecMsgCtrl *instance_p);

void FCodecStop(FCodecMsgCtrl *instance_p);

void FCodecSelect(FCodecMsgCtrl *instance_p);

#ifdef __cplusplus
}
#endif
#endif