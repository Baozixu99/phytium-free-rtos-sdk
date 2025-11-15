/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fsdif_timing.c
 * Date: 2023-07-25 14:53:42
 * LastEditTime: 2023-07-25 08:25:29
 * Description:  This file is for sdif timing function
 *
 * Modify History:
 *  Ver   Who        Date       Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/9/25   init commit
 */

#include "fparameters.h"
#include "fassert.h"
#include "fiopad.h"
#include "fsdif_timing.h"
#include "fio_mux.h"

#define FSDIF_0_SD_CCLK_OUT_DELAY FIOPAD_AJ45_REG1_OFFSET
#define FSDIF_1_SD_CCLK_OUT_DELAY FIOPAD_J53_REG1_OFFSET

extern FIOPadCtrl iopad_ctrl;

static void FSdifSetSDIFDelay(u32 sdif_id)
{
    /*
        for SD-0: 0x32b31120 = 0x1f00
            SD-1: 0x32b31178 = 0x1f00
    */
    FASSERT(iopad_ctrl.is_ready);
    u32 reg;

    if (FSDIF0_ID == sdif_id)
    {
        reg = FSDIF_0_SD_CCLK_OUT_DELAY;
    }
    else
    {
        reg = FSDIF_1_SD_CCLK_OUT_DELAY;
    }

    FIOPadSetDelay(&iopad_ctrl, reg, FIOPAD_OUTPUT_DELAY, FIOPAD_DELAY_COARSE_TUNING, FIOPAD_DELAY_1);
    FIOPadSetDelay(&iopad_ctrl, reg, FIOPAD_OUTPUT_DELAY, FIOPAD_DELAY_FINE_TUNING, FIOPAD_DELAY_7);
    FIOPadSetDelayEn(&iopad_ctrl, reg, FIOPAD_OUTPUT_DELAY, TRUE);
}

static void FSdifUnsetSDIFDelay(u32 sdif_id)
{
    /*
        for SD-0: 0x32b31120 = 0x0
            SD-1: 0x32b31178 = 0x0
    */
    FASSERT(iopad_ctrl.is_ready);
    u32 reg;

    if (FSDIF0_ID == sdif_id)
    {
        reg = FSDIF_0_SD_CCLK_OUT_DELAY;
    }
    else
    {
        reg = FSDIF_1_SD_CCLK_OUT_DELAY;
    }

    FIOPadSetDelay(&iopad_ctrl, reg, FIOPAD_OUTPUT_DELAY, FIOPAD_DELAY_COARSE_TUNING, FIOPAD_DELAY_NONE);
    FIOPadSetDelay(&iopad_ctrl, reg, FIOPAD_OUTPUT_DELAY, FIOPAD_DELAY_FINE_TUNING, FIOPAD_DELAY_NONE);
    FIOPadSetDelayEn(&iopad_ctrl, reg, FIOPAD_OUTPUT_DELAY, FALSE);
}

/* default timing settings in e2000q demo board */
static FSdifTiming mmc_sd_400khz = {
    .use_hold = 1,
    .clk_div = 0x7e7dfa,
    .clk_src = 0x000502,
    .shift = 0x0,
    .pad_delay = FSdifUnsetSDIFDelay,
};

static FSdifTiming sd_25mhz = {
    .use_hold = 1,
    .clk_div = 0x030204,
    .clk_src = 0x000302,
    .shift = 0x0,
    .pad_delay = FSdifUnsetSDIFDelay,
};

static FSdifTiming sd_50mhz = {
    .use_hold = 1,
    .clk_div = 0x030204,
    .clk_src = 0x000502,
    .shift = 0x0,
    .pad_delay = FSdifSetSDIFDelay,
};

static FSdifTiming sd_100mhz = {
    .use_hold = 0,
    .clk_div = 0x010002,
    .clk_src = 0x000202,
    .shift = 0x0,
    .pad_delay = FSdifSetSDIFDelay,
};

static FSdifTiming mmc_26mhz = {
    .use_hold = 1,
    .clk_div = 0x030204,
    .clk_src = 0x000302,
    .shift = 0x0,
    .pad_delay = FSdifSetSDIFDelay,
};

static FSdifTiming mmc_52mhz = {
    .use_hold = 0,
    .clk_div = 0x030204,
    .clk_src = 0x000202,
    .shift = 0x0,
    .pad_delay = FSdifSetSDIFDelay,
};

static FSdifTiming mmc_66mhz = {
    .use_hold = 0,
    .clk_div = 0x010002,
    .clk_src = 0x000202,
    .shift = 0x0,
    .pad_delay = NULL,
};

static FSdifTiming mmc_100mhz = {
    .use_hold = 0,
    .clk_div = 0x010002,
    .clk_src = 0x000202,
    .shift = 0x0,
    .pad_delay = FSdifSetSDIFDelay,
};

FError FSdifGetTimingSetting(FSdifClkSpeed clock_freq, boolean non_removable, FSdifTiming *tuning)
{
    FASSERT(tuning);
    FError err = FSDIF_SUCCESS;

    if (clock_freq == FSDIF_CLK_SPEED_400KHZ)
    {
        *tuning = mmc_sd_400khz;
        return err;
    }

    if (non_removable)
    {
        /* for emmc card */
        switch (clock_freq)
        {
            case FSDIF_CLK_SPEED_26_MHZ:
                *tuning = mmc_26mhz;
                break;
            case FSDIF_CLK_SPEED_52_MHZ:
                *tuning = mmc_52mhz;
                break;
            case FSDIF_CLK_SPEED_66_MHZ:
                *tuning = mmc_66mhz;
                break;
            case FSDIF_CLK_SPEED_100_MHZ:
                *tuning = mmc_100mhz;
                break;
            /* TODO: more clock freq for tuning */
            default:
                err = FSDIF_ERR_NOT_SUPPORT;
                break;
        }
    }
    else
    {
        /* for sd card */
        switch (clock_freq)
        {
            case FSDIF_CLK_SPEED_25_MHZ:
                *tuning = sd_25mhz;
                break;
            case FSDIF_CLK_SPEED_50_MHZ:
                *tuning = sd_50mhz;
                break;
            case FSDIF_CLK_SPEED_100_MHZ:
                *tuning = sd_100mhz;
                break;
            /* TODO: more clock freq for tuning */
            default:
                err = FSDIF_ERR_NOT_SUPPORT;
                break;
        }
    }

    return err;
}

void FSdifTimingInit(void)
{
    FIOMuxInit();
}

void FSdifTimingDeinit(void)
{
    FIOMuxDeInit();
}