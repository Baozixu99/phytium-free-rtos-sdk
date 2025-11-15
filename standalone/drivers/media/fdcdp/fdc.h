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
 * FilePath: fdc.h
 * Date: 2024-09-19 14:53:42
 * LastEditTime: 2024-09-19 08:29:10
 * Description:  This file is for providing some hardware register and function definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2024/09/19   Modify the format and establish the version
 */

#ifndef FDC_H
#define FDC_H

#include "ftypes.h"
#include "ferror_code.h"

#define FDC_CRTC_SCAL_ERR FT_MAKE_ERRCODE(ErrModBsp, ErrBspMEDIA, 2)
#define FDC_PIXEL_ERR     FT_MAKE_ERRCODE(ErrModBsp, ErrBspMEDIA, 3)
/************************** Constant Definitions *****************************/
#define FDC_SUCCESS       FT_SUCCESS
#define FDC_FAILED        1

#define GAMMA_INDEX_MAX   256

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
    FDC_RESET_CORE = 0,
    FDC_RESET_AXI,
    FDC_RESET_AHB,
} FDcRestType;

typedef enum
{
    FDC_MULTI_MODE_CLONE = 0,
    FDC_MULTI_MODE_HORIZONTAL,
    FDC_MULTI_MODE_VERTICAL,

    FDC_MULTI_MODE
} FDcMultiMode;

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
    boolean scale_enable;
    u32 bpc;
} FDcCrtc;

typedef struct
{
    u32 instance_id;          /* DC id */
    uintptr dcch_base_addr;   /* DC channel register address*/
    uintptr dcctrl_base_addr; /* DC control register address */
    u32 irq_num;              /* Device interrupt id */
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
    FDcDisplayVideoMode display_mode;
    FDcCrtc crtc;
    FDcTimming timming;
    FDcConfig config;
    u32 channel;
    uintptr fb_addr;
    uintptr fb_virtual;
    u32 multi_mode; /* The display mode of the device , including clone, horizontal and vertical display*/
    u32 timing_id;
    u32 pixel_clock;
    u32 color_depth;
} FDcCtrl;

void FDcHwDisable(FDcCtrl *instance, u32 channel);

void FDcHwReset(FDcCtrl *instance);

u32 FDcResToPixelClock(u32 timing_id);

FError FDcModeChangeConfigBegin(FDcCtrl *instance_p);

void FDcHwAhbReset(FDcCtrl *instance_p, u32 num, FDcRestType type);

FError FDcHwConfigPixelClock(FDcCtrl *insance, int clock);

void FDcHwSetFbAddr(FDcCtrl *instance);

FError FDcResToModeNum(FDcCtrl *instance_p, u32 width, u32 height);
#endif /* FDC_H */