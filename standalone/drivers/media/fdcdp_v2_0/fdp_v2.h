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
 * FilePath: fdp_v2.h
 * Created Date: 2025-02-06 10:36:59
 * Last Modified: 2025-07-08 16:28:02
 * Description:  This file is for
 *
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0      wzq         2025/07/08         first version
 */

#include "ftypes.h"
#include "ferror_code.h"

/************************** Constant Definitions *****************************/
#define FDP_SUCCESS FT_SUCCESS
#define FDP_FAILED  1

typedef enum
{
    FDP_PHY_TP_NONE = 0,
    FDP_PHY_TP_D10_2,
    FDP_PHY_TP_SYMBOL_ERROR,
    FDP_PHY_TP_PRBS7,
    FDP_PHY_TP_80BIT_CUSTOM,
    FDP_PHY_TP_CP2520_1,
    FDP_PHY_TP_CP2520_2,
    FDP_PHY_TP_CP2520_3,
} FDpDpcdType;

typedef enum
{
    FDP_TRAINING_OFF = 0,
    FDP_TRAINING_PATTERN_1,
    FDP_TRAINING_PATTERN_2,
    FDP_TRAINING_PATTERN_3
} FDpTrainPattern;

typedef enum
{
    FDP_BIT_DEPTH_6 = 0,
    FDP_BIT_DEPTH_8,
    FDP_BIT_DEPTH_10,
    FDP_BIT_DEPTH_12,
    FDP_BIT_DEPTH_16,
    FDP_BIT_DEPTH_RESERVE
} FDpBitDepth;

typedef struct
{
    u32 sample_rate;
    u32 link_rate;
    u16 m;
    u16 n;
} FDcDpAudioMN;

enum
{
    FDP_MODE_640_480 = 0,
    FDP_MODE_800_600,
    FDP_MODE_1024_768,
    FDP_MODE_1280_720,
    FDP_MODE_1366_768,
    FDP_MODE_1920_1080,
    FDP_MODE_1600_1200,
    FDP_MODE_1280_800,
    FDP_MODE_800_480,
    FDP_MODE_1280_1024,
    FDP_MODE_2560_1600,
    FDP_MODE_2880_1800,
    FDP_MODE_1920_1200,
    FDC_GOP_MAX_MODENUM
};

typedef struct
{
    u32 crtc_htotal;
    u32 crtc_hdisplay;
    u32 crtc_hsync_start;
    u32 crtc_hsync_end;
    boolean sync_polarity_h;
    u32 crtc_vtotal;
    u32 crtc_vdisplay;
    u32 crtc_vsync_start;
    u32 crtc_vsync_end;
    boolean sync_polarity_v;
} FDpTimming;

typedef struct
{
    u32 instance_id; /* Dp id */
    uintptr dp_channe_base_addr;
    uintptr dp_phy_addr;
    u32 irq_num; /* Device interrupt id */
} FDpConfig;

typedef struct
{
    u32 link_rate;
    u32 link_lane_count;
    u32 pixel_clock;
    u32 bit_depth;
    u8 color_rep_format; /* Color representation format. 0 - RGB , 1 - YCbCr 4:2:2 , 2 - YCbCr 4:4:4 , other - Reserved. */
    u8 clock_mode; /* clock_mode   Clocking mode for the user data. 0 - asynchronous clock , 1 - synchronous clock. */
    u32 phy_mode;  /* phy_mode     PHY mode for the user data */
} FDpTransConfig;

typedef struct
{
    boolean backlight_enabled;
    boolean power_enabled;
    boolean pwm_enabled;
    unsigned int min;
    unsigned int level;
    unsigned int max;
    u32 panel_power_up_delay;
    u32 backlight_on_delay;
    u32 backlight_off_delay;
    u32 panel_power_down_delay;
    u32 panel_power_cycle_delay;
    u32 duty_cycle;
} FDpPanel;

typedef struct
{
    u32 width;
    u32 height;
    u32 color_depth;
    u32 refresh_rate;
    u32 mode_name;
} FDpGopMode;

typedef struct
{
    u32 sample_rate;  /*the sample rate of audio */
    u32 sample_width; /*the sample width of audio*/
    u32 link_rate;    /*the link rate of media audio*/
    u16 m;            /*the audio freq of Dp */
    u16 n;            /*the audio freq of Dp */
} FDpAudioConfig;


typedef struct
{
    FDpConfig config;
    FDpPanel panel;
    /*audio config*/
    FDpAudioConfig audio_config;
    FDpTransConfig trans_config;
    FDpTimming timming;
    u32 channel;
    u32 multi_mode; /* The display mode of the device , including clone, horizontal and vertical display*/
    u32 port; /* The port of the device */
    boolean is_edp;
} FDpCtrl;


void FDpV2HwReset(FDpCtrl *instance);

FError FDpV2HwSetLaneRate(FDpCtrl *instance, u8 link_count, u32 link_rate);

FError FDpV2StartLinkTrain(FDpCtrl *instance);

u8 FDpV2TxSourcePreemphasisForValue(FDpCtrl *instance, u8 lane_num);

FError FDpV2CheckTrainingStatus(FDpCtrl *instance, u8 lane_count, u8 tpsn, u8 *value);

void FDpV2TxPhyGetAdjustRequest(FDpCtrl *instance, u8 *swing, u8 *pre_emphasis);

FError FDpV2LinkTrainingClockRecovery(FDpCtrl *instance, u8 *vswing, u8 *pre_emphasis);

FError FDpV2LinkTrainingChannelEqual(FDpCtrl *instance, u8 *vswing, u8 *pre_emphaisis);

void FDpV2HwEnableOutput(FDpCtrl *instance);

void FDpV2HwDisableOutput(FDpCtrl *instance);

void FDpV2SetCrtcEnable(FDpCtrl *instance, u32 color_depth);

const FDcDpAudioMN *FDpV2AudioGetMN(FDpCtrl *instance, u32 link_rate);

FError FDpV2AudioEnable(FDpCtrl *instance);

FError FDpV2AudioSetPara(FDpCtrl *instance);

FError FDpV2SetCrtcSet(FDpCtrl *instance);

void FDpV2InitAux(FDpCtrl *instance);

void FDpV2LinkPhyInit(FDpCtrl *instance, u32 link_rate);

void FDpV2ConfigInit(FDpCtrl *instance, u32 color_depth);

FError FDpV2WaitAuxReply(FDpCtrl *instance);

FError FDpV2SinkDpcdWrite(FDpCtrl *instance, uintptr address, u8 data);

FError FDpV2SinkDpcdRead(FDpCtrl *instance, uintptr address, u8 *data);