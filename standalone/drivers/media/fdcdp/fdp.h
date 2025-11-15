/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fdp.h
 * Date: 2024-09-19 14:53:42
 * LastEditTime: 2024-09-19 08:29:10
 * Description:  This file is for providing some hardware register and function definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2024/09/19   Modify the format and establish the version
 */

#ifndef FDP_H
#define FDP_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fdrivers_port.h"

#define FDP_PHY_INIT_ERR       FT_MAKE_ERRCODE(ErrModBsp, ErrBspMEDIA, 1)
/************************** Constant Definitions *****************************/
#define FDP_SUCCESS            FT_SUCCESS
#define FDP_FAILED             1
#define MAX_LANE_COUNT         0x00002
#define DP_MAX_SUPPORTED_RATES 4
#define DP_LINK_STATUS_SIZE    6
/* phy register start */
#define FDP_PHY_MODE_DPX4      0x01
#define FDP_PHY_MODE_DP_LANE23 0x02 // pciex2+dpx2
#define FDP_PHY_MODE_DP_LANE01 0x03 // dpx2+satax2

typedef enum
{
    FDC_FORMAT_X4R4G4B4 = 0x0,
    FDC_FORMAT_A4R4G4B4,
    FDC_FORMAT_X1R5G5B5,
    FDC_FORMAT_A1R5G5B5,
    FDC_FORMAT_R5G6B5,
    FDC_FORMAT_X8R8G8B8,
    FDC_FORMAT_A8R8G8B8,
    FDC_FORMAT_YUY2,
    FDC_FORMAT_UYVY,
    FDC_FORMAT_INDEX8,
    FDC_FORMAT_MONOCHROME,
    FDC_FORMAT_YV12 = 0x0F,
    FDC_FORMAT_A8,
    FDC_FORMAT_NV12,
    FDC_FORMAT_NV16,
    FDC_FORMAT_RG16,
    FDC_FORMAT_RB,
    FDC_FORMAT_NV12_10BIT,
    FDC_FORMAT_A2R10G10B10,
    FDC_FORMAT_NV16_10BIT,
    FDC_FORMAT_INDEX1,
    FDC_FORMAT_INDEX2,
    FDC_FORMAT_INDEX4,
    FDC_FORMAT_P010
} FDcVideoFormate;

typedef enum
{
    FDP_PWR_MODE_D0 = 1,
    FDP_PWR_MODE_D3 = 2,
} FDpPwrMode; /* the power state of dp_phy */

typedef enum
{
    FDP_BIT_DEPTH_6 = 0,
    FDP_BIT_DEPTH_8,
    FDP_BIT_DEPTH_10,
    FDP_BIT_DEPTH_12,
    FDP_BIT_DEPTH_16,
    FDP_BIT_DEPTH_RESERVE
} FDpBitDepth;

typedef enum
{
    FDP_TRAINING_OFF = 0,
    FDP_TRAINING_PATTERN_1,
    FDP_TRAINING_PATTERN_2,
    FDP_TRAINING_PATTERN_3
} FDpTrainPattern;

enum phytium_dpcd_phy_tp
{
    PHYTIUM_PHY_TP_NONE = 0,
    PHYTIUM_PHY_TP_D10_2,
    PHYTIUM_PHY_TP_SYMBOL_ERROR,
    PHYTIUM_PHY_TP_PRBS7,
    PHYTIUM_PHY_TP_80BIT_CUSTOM,
    PHYTIUM_PHY_TP_CP2520_1,
    PHYTIUM_PHY_TP_CP2520_2,
    PHYTIUM_PHY_TP_CP2520_3,
};

typedef struct
{
    u32 instance_id; /* Dp id */
    uintptr dp_channe_base_addr;
    uintptr dp_phy_base_addr;
    u32 irq_num; /* Device interrupt id */
} FDpConfig;

typedef struct
{
    unsigned int address;
    u8 request;
    u8 reply;
    void *buffer;
    size_t size;
} FDpAuxConfig;

typedef struct
{
    u32 link_rate;
    u32 link_lane_count;
    u32 pixel_clock;
    u32 bit_depth;
    u8 color_rep_format; /* Color representation format. 0 - RGB , 1 - YCbCr 4:2:2 , 2 - YCbCr 4:4:4 , other - Reserved. */
    u8 clock_mode; /* clock_mode   Clocking mode for the user data. 0 - asynchronous clock , 1 - synchronous clock. */
    u8 phy_mode;   /* phy_mode     PHY mode for the user data */
} FDpTransConfig;

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
    u32 sample_rate;  /*the sample rate of audio */
    u32 sample_width; /*the sample width of audio*/
    u32 link_rate;    /*the link rate of media audio*/
    u16 m;            /*the audio freq of Dp */
    u16 n;            /*the audio freq of Dp */
} FDpAudioConfig;

typedef struct
{
    FDpAuxConfig aux;
    FDpConfig config;
    FDpTransConfig trans_config;
    /*audio config*/
    FDpAudioConfig audio_config;
    FDpTimming timming;
    u32 channel;
    u32 multi_mode; /* The display mode of the device , including clone, horizontal and vertical display*/
    u32 port; /* The port of the device */
} FDpCtrl;

void FDpInitAux(FDpCtrl *instance_p);

void FDpLinkPhyInit(FDpCtrl *instance_p, u32 link_rate);

FError FDpStartLinkTraining(FDpCtrl *instance);

void FDpHwEnableOutput(FDpCtrl *instance);

void FDpHwScramblerReset(FDpCtrl *instance);

void FDpHwReset(FDpCtrl *instance);

FError FDpHwSetLaneRate(FDpCtrl *instance);

FError FDptxPhyGetLaneCount(FDpCtrl *instance, u8 *lane_count);

FError FDpTxPhyGetLinkRate(FDpCtrl *instance, u32 *link_rate);

FError FDpLinkTrainingClockRecovery(FDpCtrl *instance, u8 *lane_count, u32 *link_rate,
                                    u8 *vswing, u8 *pre_emphasis);
#endif /* FDP_H */