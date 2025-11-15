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
 * FilePath: fdc.c
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
#include "ftypes.h"
#include "fdrivers_port.h"
#include "fassert.h"

#include "fdc.h"
#include "fdc_hw.h"
#include "fdcdp_reg.h"
/***************************** Macro Definitions **********/
#define FDC_DEBUG_TAG          "FDC"
#define FDC_ERROR(format, ...) FT_DEBUG_PRINT_E(FDC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDC_INFO(format, ...)  FT_DEBUG_PRINT_I(FDC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDC_DEBUG(format, ...) FT_DEBUG_PRINT_D(FDC_DEBUG_TAG, format, ##__VA_ARGS__)

/***************************** Function Prototypes **********/
/**
 * @name: FDcModeChangeConfigBegin
 * @msg:  change the mode of the display, begin the mode change process
 * @param {FDcCtrl *} instance_p is the instance of the dc 
 * @return Null
 */
FError FDcModeChangeConfigBegin(FDcCtrl *instance_p)
{
    FASSERT(instance_p != NULL);
    FError ret = FDC_SUCCESS;
    u32 config;
    u32 timeout = 20;
    uintptr address = instance_p->config.dcch_base_addr;

    config = FDcChannelRegRead(address, PHYTIUM_DC_FRAMEBUFFER_CONFIG);
    if (config & FRAMEBUFFER_RESET)
    {
        do
        {
            FDC_INFO("Timeout = %d", timeout);
            config = FDcChannelRegRead(address, PHYTIUM_DC_FRAMEBUFFER_CONFIG);
            if (config & FRAMEBUFFER_PROGRESS)
            {
                timeout--;
            }
            else
            {
                break;
            }
        } while (timeout);
        if (!timeout)
        {
            FDC_ERROR("Error ::wait dp(%d) flip timeout");
            return FDC_FAILED;
        }
        config = FDcChannelRegRead(address, PHYTIUM_DC_FRAMEBUFFER_CONFIG);

        FDC_INFO("ModeChangeConfig valid config = 0x%x", config);
        FDcChannelRegWrite(address, PHYTIUM_DC_FRAMEBUFFER_CONFIG, config | FRAMEBUFFER_VALID_PENDING);
    }
    return ret;
}

/**
 * @name: FDcHwAhbReset
 * @msg:  reset the ahb bus
 * @param {FDcCtrl *} instance_p is the instance of the dc
 * @param {u32} num is the channel number
 * @param {FDcRestType} type is the reset type
 * @return Null
 */
void FDcHwAhbReset(FDcCtrl *instance_p, u32 num, FDcRestType type)
{
    FASSERT(instance_p != NULL);
    u32 value1 = 0;
    u32 value2 = 0;
    uintptr address = instance_p->config.dcctrl_base_addr;

    value1 = FDcChannelRegRead(address, PHYTIUM_AQ_HI_CLOCK_CONTROL);
    switch (type)
    {
        case FDC_RESET_CORE:
            if (num == 0)
            {
                value1 |= DC0_CORE_RESET;
                value2 = value1 & (~DC0_CORE_RESET);
            }
            else
            {
                value1 |= DC1_CORE_RESET;
                value2 = value1 & (~DC1_CORE_RESET);
            }
            break;
        case FDC_RESET_AXI:
            value1 |= AXI_RESET;
            value2 = value1 & (~AXI_RESET);
            break;
        case FDC_RESET_AHB:
            value1 |= AHB_RESET;
            value2 = value1 & (~AHB_RESET);
            break;
        default:
            FDC_ERROR("Dc Reset Type Error!");
            break;
    }
    FDcChannelRegWrite(address, PHYTIUM_AQ_HI_CLOCK_CONTROL, value1);
    FDriverMdelay(20);
    FDcChannelRegWrite(address, PHYTIUM_AQ_HI_CLOCK_CONTROL, value2);
    return;
}

/**
 * @name: FDcHwDisable
 * @msg:  disable the dc channel
 * @param {FDcCtrl *} instance_p is the instance of the dc
 * @param {u32} channel is the channel number
 * @return Null
 */
void FDcHwDisable(FDcCtrl *instance, u32 channel)
{
    int config = 0;
    uintptr dcctrl_base_addr = instance->config.dcctrl_base_addr;
    uintptr dcch_base_addr = instance->config.dcch_base_addr;
    /* clear framebuffer */
    FDcChannelRegWrite(dcch_base_addr, PHYTIUM_DC_FRAMEBUFFER_CLEARVALUE, CLEAR_VALUE_BLACK);
    config = FDcChannelRegRead(dcch_base_addr, PHYTIUM_DC_FRAMEBUFFER_CONFIG);
    config |= FRAMEBUFFER_CLEAR;
    FDcChannelRegWrite(dcch_base_addr, PHYTIUM_DC_FRAMEBUFFER_CONFIG, config);

    /* disable cursor */
    config = FDcChannelRegRead(dcch_base_addr, PHYTIUM_DC_CURSOR_CONFIG);
    config = ((config & (~CURSOR_FORMAT_MASK)) | CURSOR_FORMAT_DISABLED);
    FDcChannelRegWrite(dcch_base_addr, PHYTIUM_DC_CURSOR_CONFIG, config);
    FDriverUdelay(20);

    /* reset pix clock */
    FDcHwConfigPixelClock(instance, 0);

    if (channel == 0)
    {
        config = FDcChannelRegRead(dcctrl_base_addr, PHYTIUM_AQ_HI_CLOCK_CONTROL);
        FDcChannelRegWrite(dcctrl_base_addr, PHYTIUM_AQ_HI_CLOCK_CONTROL, config | DC0_CORE_RESET);
        FDriverUdelay(20);
        FDcChannelRegWrite(dcctrl_base_addr, PHYTIUM_AQ_HI_CLOCK_CONTROL, config & (~DC0_CORE_RESET));
    }
    else
    {
        config = FDcChannelRegRead(dcctrl_base_addr, PHYTIUM_AQ_HI_CLOCK_CONTROL);
        FDcChannelRegWrite(dcctrl_base_addr, PHYTIUM_AQ_HI_CLOCK_CONTROL, config | DC1_CORE_RESET);
        FDriverUdelay(20);
        FDcChannelRegWrite(dcctrl_base_addr, PHYTIUM_AQ_HI_CLOCK_CONTROL, config & (~DC1_CORE_RESET));
    }
    FDriverUdelay(20);
}

/**
 * @name: FDcHwDisable
 * @msg:  disable the dc channel
 * @param {FDcCtrl *} instance_p is the instance of the dc
 * @param {u32} width is the width of the display
 * @param {u32} height is the height of the display
 * @return type of the display
 */
FError FDcResToModeNum(FDcCtrl *instance_p, u32 width, u32 height)
{

    u32 type = 0;

    if ((width == 640) && (height == 480))
    {
        type = FDC_DISPLAY_ID_640_480;
    }
    else if ((width == 800) && (height == 600))
    {
        type = FDC_DISPLAY_ID_800_600;
    }
    else if ((width == 1024) && (height == 768))
    {
        type = FDC_DISPLAY_ID_1024_768;
    }
    else if ((width == 1280) && (height == 720))
    {
        type = FDC_DISPLAY_ID_1280_720;
    }
    else if ((width == 1366) && (height == 768))
    {
        type = FDC_DISPLAY_ID_1366_768;
    }
    else if ((width == 1920) && (height == 1080))
    {
        type = FDC_DISPLAY_ID_1920_1080;
    }
    else if ((width == 1600) && (height == 1200))
    {
        type = FDC_DISPLAY_ID_1600_1200;
    }
    else if ((width == 1280) && (height == 800))
    {
        type = FDC_DISPLAY_ID_1280_800;
    }
    else if ((width == 800) && (height == 480))
    {
        type = FDC_DISPLAY_ID_800_480;
    }
    else if ((width == 1280) && (height == 768))
    {
        type = FDC_DISPLAY_ID_1280_768;
    }
    else if ((width == 1280) && (height == 1024))
    {
        type = FDC_DISPLAY_ID_1280_1024;
    }
    else
    {
        type = FDC_DISPLAY_ID_1024_768;
        FDC_ERROR("Unsupported resolution,use default 1024*768");
    }

    return type;
}

/**
 * @name: FDcResToPixelClock
 * @msg:  set the pixel clock of the dc channel
 * @param {u32 } timing_id is the timing id of the display
 * @return pixel clock of the display
 */
u32 FDcResToPixelClock(u32 timing_id)
{
    int pixel_clock = 0;
    switch (timing_id)
    {
        case 0:
            pixel_clock = 25175;
            break;
        case 1:
            pixel_clock = 40000;
            break;
        case 2:
            pixel_clock = 65000;
            break;
        case 3:
            pixel_clock = 74250;
            break;
        case 4:
            pixel_clock = 85500;
            break;
        case 5:
            pixel_clock = 148500;
            break;
        case 6:
            pixel_clock = 162000;
            break;
        case 7:
            pixel_clock = 83500;
            break;
        case 8:
            pixel_clock = 28232;
            break;
        case 9:
            pixel_clock = 108000;
            break;
        default:
            FDC_ERROR("the pixel_clock is in invalid,use the 25175");
            pixel_clock = 25175;
            break;
    }
    return pixel_clock;
}

/**
 * @name: FDcHwConfigPixelClock
 * @msg:  set the pixel clock of the dc channel
 * @param {FDcCtrl *} instance is the instance of the display
 * @param {int} clock is the pixel clock of the display
 * @return return FDC_SUCCESS or FDC_FAILED
 */
FError FDcHwConfigPixelClock(FDcCtrl *instance, int clock)
{
    FError ret = FDC_SUCCESS;
    uintptr addr = instance->config.dcctrl_base_addr;
    u32 value = 0;
    u32 check_times;
    u32 check_times1;
    uintptr reg = 0;
    switch (instance->channel)
    {
        case 0:
            reg = PHYTIUM_CTRL_CH0_PIXEL_CLOCK(0);
            break;
        case 1:
            reg = PHYTIUM_CTRL_CH0_PIXEL_CLOCK(1);
            break;
        default:
            FASSERT(0);
            break;
    }
    check_times = 1000;
    value &= PHYTIUM_CTRL_PIXEL_CLOCK_CLEAR_MASK;
    value |= clock;
    value |= PHYTIUM_CTRL_PIXEL_CLOCK_ENABLE;
    FDcChannelRegWrite(addr, reg, value);
    while (check_times != 0)
    {
        value = FDcChannelRegRead(addr, reg);
        if ((value & PHYTIUM_CTRL_PIXEL_CLOCK_VALID) != 0)
        {
            /* clear bit 30 */
            value &= (~PHYTIUM_CTRL_PIXEL_CLOCK_ENABLE);
            FDcChannelRegWrite(addr, reg, value);

            for (check_times1 = 0; check_times1 < 500; check_times1++)
            {
                value = FDcChannelRegRead(addr, reg);
                if ((value & PHYTIUM_CTRL_PIXEL_CLOCK_VALID) == 0)
                {
                    ret = FDC_SUCCESS;
                    break;
                }
            }
            break;
        }
        check_times--;
        if (check_times == 0)
        {
            value &= (~PHYTIUM_CTRL_PIXEL_CLOCK_ENABLE);
            FDcChannelRegWrite(addr, reg, value);
            FDC_ERROR("Pixel clock change timeout");
            ret = -1;
        }
    }
    return ret;
}

/**
 * @name: FDcHwSetFbAddr
 * @msg:  set the FB address of the dc channel
 * @param {FDcCtrl *} instance is the instance of the display
 * @return 
 */
void FDcHwSetFbAddr(FDcCtrl *instance)
{
    uintptr addr = instance->config.dcch_base_addr;
    u32 data = FDcChannelRegRead(addr, PHYTIUM_DC_PANEL_CONFIG);

    /*set the panel params*/
    data |= PANEL_CLOCK_ENABLE;
    data |= PANEL_DATA_ENABLE;
    data |= PANEL_DATAENABLE_ENABLE;
    FDcChannelRegWrite(addr, PHYTIUM_DC_FRAMEBUFFER_Y_ADDRESS, (u32)instance->fb_virtual);
    FDcChannelRegWrite(addr, PHYTIUM_DC_FRAMEBUFFER_X_ADDRESS, 0x0U);
    FDcChannelRegWrite(addr, PHYTIUM_DC_PANEL_CONFIG, data);
    return;
}

/**
 * @name: FDcHwReset
 * @msg:  reset  the dc channel
 * @param {FDcCtrl *} instance is the instance of the display
 * @return 
 */
void FDcHwReset(FDcCtrl *instance)
{
    u32 channel = instance->channel;
    uintptr addr = instance->config.dcctrl_base_addr;
    int config = 0;
    /* disable pixel clock for bmc mode */
    if (channel == 0)
    {
        FDcHwDisable(instance, channel);
    }

    config = FDcChannelRegRead(addr, PHYTIUM_AQ_HI_CLOCK_CONTROL);
    config &= (~(DC0_CORE_RESET | DC1_CORE_RESET | AXI_RESET | AHB_RESET));

    if (channel == 0)
    {
        FDcChannelRegWrite(addr, PHYTIUM_AQ_HI_CLOCK_CONTROL, config | DC0_CORE_RESET);
        FDriverUdelay(20);
        FDcChannelRegWrite(addr, PHYTIUM_AQ_HI_CLOCK_CONTROL, config | DC0_CORE_RESET | AXI_RESET);
        FDriverUdelay(20);
        FDcChannelRegWrite(addr, PHYTIUM_AQ_HI_CLOCK_CONTROL,
                           config | DC0_CORE_RESET | AXI_RESET | AHB_RESET);
        FDriverUdelay(20);
        FDcChannelRegWrite(addr, PHYTIUM_AQ_HI_CLOCK_CONTROL, config | DC0_CORE_RESET | AXI_RESET);
        FDriverUdelay(20);
        FDcChannelRegWrite(addr, PHYTIUM_AQ_HI_CLOCK_CONTROL, config | DC0_CORE_RESET);
        FDriverUdelay(20);
        FDcChannelRegWrite(addr, PHYTIUM_AQ_HI_CLOCK_CONTROL, config);
        FDriverUdelay(20);
    }
    else
    {
        FDcChannelRegWrite(addr, PHYTIUM_AQ_HI_CLOCK_CONTROL, config | DC1_CORE_RESET);
        FDriverUdelay(20);
        FDcChannelRegWrite(addr, PHYTIUM_AQ_HI_CLOCK_CONTROL, config | DC1_CORE_RESET | AXI_RESET);
        FDriverUdelay(20);
        FDcChannelRegWrite(addr, PHYTIUM_AQ_HI_CLOCK_CONTROL,
                           config | DC1_CORE_RESET | AXI_RESET | AHB_RESET);
        FDriverUdelay(20);
        FDcChannelRegWrite(addr, PHYTIUM_AQ_HI_CLOCK_CONTROL, config | DC1_CORE_RESET | AXI_RESET);
        FDriverUdelay(20);
        FDcChannelRegWrite(addr, PHYTIUM_AQ_HI_CLOCK_CONTROL, config | DC1_CORE_RESET);
        FDriverUdelay(20);
        FDcChannelRegWrite(addr, PHYTIUM_AQ_HI_CLOCK_CONTROL, config);
        FDriverUdelay(20);
    }
    FDC_DEBUG("Resetting dc channel ");
}