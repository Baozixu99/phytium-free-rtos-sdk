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
 * FilePath: fdc_v2.h
 * Created Date: 2025-02-11 15:29:37
 * Last Modified: 2025-07-08 16:28:02
 * Description:  This file is for
 *
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0      wzq         2025/07/08         first version
 */
#ifndef FDC_V2_H
#define FDC_V2_H

/***************************** Include Files *********************************/
#include "ftypes.h"
#include "ferror_code.h"


#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
#define FDC_SUCCESS  FT_SUCCESS
#define FDC_FAILED   1

#define FDC_NOT_INIT FT_MAKE_ERRCODE(ErrModBsp, ErrBspMEDIAMsg, 1)
#define FDC_PARA_ERR FT_MAKE_ERRCODE(ErrModBsp, ErrBspMEDIAMsg, 2)

typedef enum
{
    FDC_DISPLAY_ID_640_480 = 0,
    FDC_DISPLAY_ID_800_600,
    FDC_DISPLAY_ID_1024_768,
    FDC_DISPLAY_ID_1280_720,
    FDC_DISPLAY_ID_1366_768,
    FDC_DISPLAY_ID_1920_1080,
    FDC_DISPLAY_ID_1600_1200,
    FDC_DISPLAY_ID_1280_800,
    FDC_DISPLAY_ID_800_480,
    FDC_DISPLAY_ID_1280_768,
    FDC_DISPLAY_ID_1280_1024,
    FDC_DISPLAY_ID_MAX_NUM

} FDcDisplayId;

typedef struct
{
    uintptr y_address;
    uintptr u_address;
    uintptr v_address;
    u32 u_stride;
    u32 v_stride;
    u32 rot_angle;
    u32 alpha_mode;
    u32 alpha_value;
    /* Original size in pixel before rotation and scale. */
    u32 tile_mode;
    u32 scale;
    u32 scale_factorx;
    u32 scale_factory;
    u32 filter_tap;
    u32 horizontal_filtertap;
#define HOR_KERNEL_MAX 128
    u32 horkernel[HOR_KERNEL_MAX];
#define VER_KERNEL_MAX 128
    u32 verkernel[VER_KERNEL_MAX];
    u32 swizzle;
    u32 uv_swizzle;
    u32 color_key;
    u32 colorkey_high;
    u32 bg_color;
    u32 trans_parency;
    u32 clear_fb;
    u32 clear_value;
    u32 initial_offsetx;
    u32 initial_offsety;
    u32 compressed;
    u32 gamma;
} FDcDisplayVideoMode;

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
    FDC_MULTI_MODE_CLONE = 0,
    FDC_MULTI_MODE_HORIZONTAL,
    FDC_MULTI_MODE_VERTICAL,

    FDC_MULTI_MODE
} FDcMultiMode;

typedef enum
{
    FDC_RESET_CORE = 0,
    FDC_RESET_AXI,
    FDC_RESET_AHB,
} FDcRestType;

typedef struct
{
    u32 instance_id;      /* DC id */
    uintptr dc_base_addr; /* DC control register address */
    u32 irq_num;          /* Device interrupt id */
} FDcConfig;

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
} FDcTimming;

typedef struct
{
    boolean scale_enable;
    u32 bpc;
} FDcCrtc;
typedef struct
{
    FDcConfig config;
    FDcCrtc crtc;
    FDcTimming timming;
    FDcDisplayVideoMode display_mode;
    u32 port;
    u32 channel;
    uintptr fb_addr;
    uintptr fb_virtual;
    u32 multi_mode; /* The display mode of the device , including clone, horizontal and vertical display*/
    u32 timing_id;
    u32 pixel_clock;
    u32 color_depth;
} FDcCtrl;

int FDcV2WidthToStride(u32 width, u32 color_depth, u32 multi_mode);

FError FDcV2SetCrtcEnable(FDcCtrl *instance);

FError FDcV2FramebufferConfig(FDcCtrl *instance);

void FDcV2HwSetFbAddr(FDcCtrl *instance);

void FDcV2HwAhbReset(FDcCtrl *instance, u32 num, FDcRestType type);

u32 FDcV2ResToModeNum(FDcCtrl *instance_p, u32 width, u32 height);

u32 FDcV2ResToPixelClock(u32 timing_id);

#ifdef __cplusplus
}
#endif

#endif