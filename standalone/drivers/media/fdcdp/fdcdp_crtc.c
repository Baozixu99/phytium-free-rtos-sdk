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
 * FilePath: fdcdp_crtc.c
 * Date: 2024-09-17 22:53:24
 * LastEditTime: 2024/09/19  12:5:25
 * Description:  This file is for This file is for Handling the hardware register and
 *               providing some function interface
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2024/09/19  Modify the format and establish the version
 */


/***************************** Include Files *********************************/

#include <string.h>
#include "fio.h"
#include "fdrivers_port.h"
#include "fassert.h"
#include "ftypes.h"
#include "ferror_code.h"

#include "fdc.h"
#include "fdp.h"
#include "fdc_hw.h"
#include "fdp_hw.h"
#include "fdcdp_reg.h"
#include "fdcdp_crtc.h"
/***************** Macros (Inline Functions) Definitions *********************/

#define FDCDP_CRTC_DEBUG_TAG "FDCDP_CRTC"
#define FDCDP_CRTC_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FDCDP_CRTC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDCDP_CRTC_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FDCDP_CRTC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDCDP_CRTC_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FDCDP_CRTC_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/**
 * @name: FDcWidthToStride
 * @msg:  Buffer Width conversion to stride according to 128 pixels alignment
 * @param {u32} width is the width of displayer
 * @param {u32} color_depth is the colordepth of displayer
 * @param {u32} multi_mode is is the multi_mode of displayer
 * @return stride ,memory image line span
 */
int FDcWidthToStride(u32 width, u32 color_depth, u32 multi_mode)
{
    u32 stride;
    if ((width % 128) == 0)
    {
        stride = width * (color_depth / 8);
    }
    else
    {
        stride = ((width / 128) * 128 + 128) * (color_depth / 8);
    }
    if (width == 800 || width == 1600)
    {
        stride = width * (color_depth / 8);
    }
    if (multi_mode == FDC_MULTI_MODE_HORIZONTAL)
    {
        stride = stride * 2;
    }
    return stride;
}

FError FDcSetCrtcEnable(FDcCtrl *instance)
{
    FError ret = FDC_SUCCESS; /*0-success*/
    FDcTimming timming;
    uintptr addr = instance->config.dcch_base_addr;
    u32 config = 0, stride = 0, bpc = 0;
    u32 dc_format;
    timming = instance->timming;

    if (instance->color_depth == 16)
    {
        dc_format = FDC_FORMAT_R5G6B5;
    }
    else
    {
        dc_format = FDC_FORMAT_A8R8G8B8;
    }

    stride = FDcWidthToStride(instance->timming.crtc_hdisplay, instance->color_depth,
                              instance->multi_mode);
    FDcChannelRegWrite(addr, PHYTIUM_DC_FRAMEBUFFER_Y_STRIDE, stride);
    config = ((timming.crtc_hdisplay & HDISPLAY_END_MASK) << HDISPLAY_END_SHIFT) |
             ((timming.crtc_htotal & HDISPLAY_TOTAL_MASK) << HDISPLAY_TOTAL_SHIFT);
    FDcChannelRegWrite(addr, PHYTIUM_DC_HDISPLAY, config);

    config = ((timming.crtc_hsync_start & HSYNC_START_MASK) << HSYNC_START_SHIFT) |
             ((timming.crtc_hsync_end & HSYNC_END_MASK) << HSYNC_END_SHIFT);
    config |= HSYNC_NEGATIVE;
    config |= HSYNC_PULSE_ENABLED;
    FDcChannelRegWrite(addr, PHYTIUM_DC_HSYNC, config);

    config = ((timming.crtc_vdisplay & VDISPLAY_END_MASK) << VDISPLAY_END_SHIFT) |
             ((timming.crtc_vtotal & VDISPLAY_TOTAL_MASK) << VDISPLAY_TOTAL_SHIFT);
    FDcChannelRegWrite(addr, PHYTIUM_DC_VDISPLAY, config);

    config = ((timming.crtc_vsync_start & VSYNC_START_MASK) << VSYNC_START_SHIFT) |
             ((timming.crtc_vsync_end & VSYNC_END_MASK) << VSYNC_END_SHIFT);
    config |= VSYNC_NEGATIVE;
    config |= VSYNC_PULSE_ENABLED;
    FDcChannelRegWrite(addr, PHYTIUM_DC_VSYNC, config);

    config = PANEL_DATAENABLE_ENABLE | PANEL_DATA_ENABLE | PANEL_CLOCK_ENABLE;
    FDcChannelRegWrite(addr, PHYTIUM_DC_PANEL_CONFIG, config);

    switch (instance->crtc.bpc)
    {
        case 10:
            bpc = DP_RGB101010;
            break;
        case 6:
            bpc = DP_RGB666;
            break;
        default:
            bpc = DP_RGB888;
            break;
    }
    config = bpc | OUTPUT_DP;
    FDcChannelRegWrite(addr, PHYTIUM_DC_DP_CONFIG, config);

    config = FDcChannelRegRead(addr, PHYTIUM_DC_FRAMEBUFFER_CONFIG);

    if (instance->crtc.scale_enable)
    {
        config |= FRAMEBUFFER_SCALE_ENABLE;
    }
    else
    {
        config &= ~FRAMEBUFFER_SCALE_ENABLE;
    }
    config |= FRAMEBUFFER_CONFIG0_ROT_ANGLE_SET(instance->display_mode.rot_angle);
    config |= FRAMEBUFFER_CONFIG0_TILE_MODE_SET(instance->display_mode.tile_mode);
    config |= FRAMEBUFFER_CONFIG0_FORMAT_SET(dc_format);
    config |= (instance->display_mode.scale & FRAMEBUFFER_SCALE_ENABLE); /* scale :1-enable,0-disble;*/
    config |= (instance->display_mode.uv_swizzle & FRAMEBUFFER_CONFIG0_UV_SWIZZLE); /* reserve */
    config |= FRAMEBUFFER_CONFIG0_SWIZZLE_SET(instance->display_mode.swizzle);
    config |= FRAMEBUFFER_CONFIG0_TRANSPARENCY_SET(instance->display_mode.trans_parency);
    config |= (instance->display_mode.clear_fb & FRAMEBUFFER_CLEAR);
    config &= (~FRAMEBUFFER_VALID_PENDING);
    config |= (instance->display_mode.gamma & FRAMEBUFFER_GAMMA_ENABLE);
    config |= FRAMEBUFFER_RESET | FRAMEBUFFER_OUTPUT;

    FDcChannelRegWrite(addr, PHYTIUM_DC_FRAMEBUFFER_CONFIG, config);

    config = FDcChannelRegRead(addr, PHYTIUM_DC_FRAMEBUFFER_SIZE);

    config = ((timming.crtc_hdisplay & WIDTH_MASK) << WIDTH_SHIFT) |
             ((timming.crtc_vdisplay & HEIGHT_MASK) << HEIGHT_SHIFT);
    FDcChannelRegWrite(addr, PHYTIUM_DC_FRAMEBUFFER_SIZE, config);

    return ret;
}

static u8 FDpSetMainStreamMisc0(u8 bit_depth, u8 color_format, u8 clock_mode)
{
    u8 value;
    FDpBitDepth BitDep;
    switch (bit_depth)
    {
        case 6:
            BitDep = FDP_BIT_DEPTH_6;
            break;
        case 8:
            BitDep = FDP_BIT_DEPTH_8;
            break;
        case 10:
            BitDep = FDP_BIT_DEPTH_10;
            break;
        case 12:
            BitDep = FDP_BIT_DEPTH_12;
            break;
        case 16:
            BitDep = FDP_BIT_DEPTH_16;
            break;
        default:
            FDCDP_CRTC_DEBUG("The bit_depth is invalid ");
            BitDep = FDP_BIT_DEPTH_RESERVE;
            break;
    }
    value = 0;
    /*bit 0 */
    value |= MISC0_SYNCHRONOUS_CLOCK;
    value |= clock_mode;
    /*bit 2:1 */
    value &= (~MISC0_COLOR_FORMAT);
    value |= (color_format << 1);
    /*bit 7:5 */
    value &= (~MISC0_BIT_DEPTH);
    value |= (BitDep << 5);

    return value;
}

FError FDpSetCrtcEnable(FDpCtrl *instance)
{
    FError ret = FDC_SUCCESS; /*0-success*/
    uintptr addr = instance->config.dp_channe_base_addr;
    FDpTimming timming;
    timming = instance->timming;
    u32 tu_size = 64;
    u32 temp;
    u32 value;
    u32 data_per_tu;
    int symbol_count, udc;

    FDpChannelRegWrite(addr, PHYTIUM_DP_MAIN_LINK_HTOTAL, timming.crtc_htotal);
    FDpChannelRegWrite(addr, PHYTIUM_DP_MAIN_LINK_VTOTAL, timming.crtc_vtotal);

    value = FDpChannelRegRead(addr, PHYTIUM_DP_MAIN_LINK_POLARITY);
    if (timming.sync_polarity_h == 0)
    {
        value &= 0xfffe;
    }
    else
    {
        value |= 0x1;
    }
    if (timming.sync_polarity_v == 0)
    {
        value &= 0xfffd;
    }
    else
    {
        value |= 0x2;
    }

    FDpChannelRegWrite(addr, PHYTIUM_DP_MAIN_LINK_POLARITY, value);
    FDpChannelRegWrite(addr, PHYTIUM_DP_MAIN_LINK_HSWIDTH, timming.crtc_hsync_start); /*hsWidth*/
    FDpChannelRegWrite(addr, PHYTIUM_DP_MAIN_LINK_VSWIDTH, timming.crtc_vsync_start); /*vsWidth*/
    FDpChannelRegWrite(addr, PHYTIUM_DP_MAIN_LINK_HRES, timming.crtc_hdisplay); /*hRes*/
    FDpChannelRegWrite(addr, PHYTIUM_DP_MAIN_LINK_VRES, timming.crtc_vdisplay); /*vRes*/
    FDpChannelRegWrite(addr, PHYTIUM_DP_MAIN_LINK_HSTART, timming.crtc_hsync_end); /*hStart*/
    FDpChannelRegWrite(addr, PHYTIUM_DP_MAIN_LINK_VSTART, timming.crtc_vsync_end);

    FDpChannelRegWrite(addr, PHYTIUM_DP_MAIN_LINK_MISC0,
                       FDpSetMainStreamMisc0(instance->trans_config.bit_depth,
                                             instance->trans_config.color_rep_format,
                                             instance->trans_config.clock_mode));
    FDpChannelRegWrite(addr, PHYTIUM_DP_M_VID, instance->trans_config.pixel_clock / 10);

    u32 links_peed = instance->trans_config.link_lane_count * instance->trans_config.link_rate / 1000;
    u32 pixel_value = instance->trans_config.pixel_clock * instance->trans_config.bit_depth * 3 / 8;
    data_per_tu = pixel_value * tu_size / links_peed;

    /*bit 6:0*/
    value |= tu_size;
    /*bit 23:16*/
    value &= 0xFF00FFFF;
    temp = (u8)(data_per_tu / 1000);
    value |= ((temp) << 16);
    /*bit 27:24*/
    temp = data_per_tu % 1000 * 16 / 1000;
    value &= 0xF0FFFFFF;
    value |= ((u8)(temp) << 24);

    FDpChannelRegWrite(addr, PHYTIUM_DP_TRANSFER_UNIT_SIZE, value);
    FDpChannelRegWrite(addr, PHYTIUM_DP_N_VID, instance->trans_config.link_rate / 10);

    symbol_count = (instance->timming.crtc_hdisplay * instance->trans_config.bit_depth * 3 + 7) / 8;
    udc = (symbol_count + instance->trans_config.link_lane_count - 1) /
          instance->trans_config.link_lane_count;

    value = (udc * 10 + 5) / 10;

    FDpChannelRegWrite(addr, PHYTIUM_DP_DATA_COUNT, udc);

    value = 0xC;
    if (instance->timming.sync_polarity_h == 0)
    {
        value &= 0xfffe;
    }
    else
    {
        value |= 0x1;
    }
    if (instance->timming.sync_polarity_v == 0)
    {
        value &= 0xfffd;
    }
    else
    {
        value |= 0x2;
    }

    FDpChannelRegWrite(addr, PHYTIUM_DP_USER_SYNC_POLARITY, value);

    return ret;
}
