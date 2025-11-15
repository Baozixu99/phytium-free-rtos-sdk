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
 * FilePath: fi2s_msg.h
 * Created Date: 2025-05-09 15:31:03
 * Last Modified: 2025-06-11 17:44:35
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */
#ifndef FI2S_MSG_H
#define FI2S_MSG_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fmsg_common.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
#define FI2S_MSG_DATA_LEN       56
#define FI2S_LSD_ID             0x00
#define FI2S_DIRECTION_PLAYBACK 0
#define FI2S_DIRECTION_CAPTURE  1
#define FI2S_MSG_SUCCESS        FT_SUCCESS

#define FI2S_MSG_NOT_INIT       FT_MAKE_ERRCODE(ErrModBsp, ErrBspI2sMsg, 1)
#define FI2S_MSG_PARA_ERR       FT_MAKE_ERRCODE(ErrModBsp, ErrBspI2sMsg, 2)
#define FI2S_MSG_CMD_ERR        FT_MAKE_ERRCODE(ErrModBsp, ErrBspI2sMsg, 3)
#define FI2S_MSG_HANDLE_ERR     FT_MAKE_ERRCODE(ErrModBsp, ErrBspI2sMsg, 4)
#define FI2S_MSG_TIME_OUT       FT_MAKE_ERRCODE(ErrModBsp, ErrBspI2sMsg, 5)
typedef enum
{
    AUDIO_PCM_STREAM_WORD_LENGTH_16 = 0, /*the trams audio length word length*/
    AUDIO_PCM_STREAM_WORD_LENGTH_20,
    AUDIO_PCM_STREAM_WORD_LENGTH_24,
    AUDIO_PCM_STREAM_WORD_LENGTH_32,
    AUDIO_PCM_STREAM_WORD_LENGTH
} FI2sPcmWordLength;

enum
{
    FI2S_MSG_CMD_SET_MODE = 0,
    FI2S_MSG_CMD_SET_TRIGGER,
    FI2S_MSG_CMD_SET_GPIO,
};

typedef enum
{
    FI2S_SAMPLE_RATE_PHONE = 8000,
    FI2S_SAMPLE_RATE_DAT = 32000,
    FI2S_SAMPLE_RATE_CD = 44100,
    FI2S_SAMPLE_RATE_DV = 48000,
    FI2S_SAMPLE_RATE_DVD = 96000,
} FI2sSampleRate;
typedef enum
{
    FI2S_COMPLETE_NONE = 0,
    FI2S_COMPLETE_DONE,
    FI2S_COMPLETE_GOING,
    FI2S_COMPLETE_ERROR = 0x10,
    FI2S_COMPLETE_ID_NOT_SUPPORTED,
    FI2S_COMPLETE_SUBID_NOT_SUPPORTED,
    FI2S_COMPLETE_INVALID_PARAMETERS,
} FI2sReportInfo;

typedef struct
{
    int direction;
    u32 chan_nr;
    u32 data_width;
    u32 sample_rate;
    u32 enable;
    u32 clk_base;
} FI2sMsgModeData;

typedef struct
{
    int direction;
    u32 start;
} FI2sMsgTriggerInfo;

typedef struct
{
    int enable;
    int insert;
} FI2sMsgGpioInfo;

typedef struct
{
    u16 seq;     /*由发起方使用或维护，对端不改变该值，默认为0*/
    u8 cmd_type; /*功能性命令字段，可以用极少位表示，当前保留为8bit*/
    u8 cmd_subid; /*子命令id，与命令实现有关，最多为256个，搭配cmd_type补充*/
    u16 len;    /*在交互数据时，表示payload的长度，交互命令时为0*/
    u16 status; /*协议命令状态，表示命令执行相应*/
    uint8_t para[FI2S_MSG_DATA_LEN];
} FI2sMsgInfo;

typedef struct
{
    u32 instance_id;
    u32 irq_num;
    u32 irq_prority;
    FDevMsg msg;
    u32 dma_reg_base;
} FI2sMsgConfig;

typedef struct
{
    FI2sMsgConfig config;
    u32 clk_base;
    FI2sMsgInfo *i2s_msg;
    int interrupt;
    int running;
    u32 chan_nr;
    u32 data_width;
    u32 sample_rate;
    int insert;
    u32 work_mode;
    u32 is_ready;
} FI2sMsgCtrl;

const FI2sMsgConfig *FI2sMsgLookupConfig(u32 instance_id);

FError FI2sMsgCfgInitialize(FI2sMsgCtrl *instance_p, const FI2sMsgConfig *input_config_p);

void FI2sMsgDeInitialize(FI2sMsgCtrl *instance_p);

FError FI2sMsgSetCmd(FI2sMsgCtrl *instance_p, FI2sMsgInfo *set_msg);

FError FI2sMsgHwSetParams(FI2sMsgCtrl *instance_p, u8 mode, u32 sample_rate, u8 chan, u8 enable);

FError FI2sMsgStart(FI2sMsgCtrl *instance_p, u8 mode, boolean enable);

#ifdef __cplusplus
}
#endif
#endif