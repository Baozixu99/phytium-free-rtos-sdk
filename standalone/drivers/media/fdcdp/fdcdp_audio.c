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
 * FilePath: fdcdp_audio.c
 * Created Date: 2023-09-11 09:19:16
 * Last Modified: 2023-11-20 14:27:12
 * Description:  This file is for
 *
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ----------------------------- ----
 */

#include <string.h>
#include <stdio.h>
#include "ftypes.h"
#include "fdrivers_port.h"
#include "ferror_code.h"
#include "fassert.h"
#include "fdp_hw.h"
#include "fdcdp_audio.h"
#include "fdp.h"
#include "fdcdp_reg.h"

#define APB_CLK         48000000
#define MAX_M_N         36
#define CHANNEL_DEFAULT 0x87654321
// clang-format off
static const FDcDpAudioMN DpAudioMN[MAX_M_N] =
{
    { 32000,  162000, 1024, 10125 },
    { 44100,  162000, 784,  5625  },
    { 48000,  162000, 512,  3375  },
    { 64000,  162000, 2048, 10125 },
    { 88200,  162000, 1568, 5625  },
    { 96000,  162000, 1024, 3375  },
    { 128000, 162000, 4096, 10125 },
    { 176400, 162000, 3136, 5625  },
    { 192000, 162000, 2048, 3375  },
    { 32000,  270000, 1024, 16875 },
    { 44100,  270000, 784,  9375  },
    { 48000,  270000, 512,  5625  },
    { 64000,  270000, 2048, 16875 },
    { 88200,  270000, 1568, 9375  },
    { 96000,  270000, 1024, 5625  },
    { 128000, 270000, 4096, 16875 },
    { 176400, 270000, 3136, 9375  },
    { 192000, 270000, 2048, 5625  },
    { 32000,  540000, 1024, 33750 },
    { 44100,  540000, 784,  18750 },
    { 48000,  540000, 512,  11250 },
    { 64000,  540000, 2048, 33750 },
    { 88200,  540000, 1568, 18750 },
    { 96000,  540000, 1024, 11250 },
    { 128000, 540000, 4096, 33750 },
    { 176400, 540000, 3136, 18750 },
    { 192000, 540000, 2048, 11250 },
    { 32000,  810000, 1024, 50625 },
    { 44100,  810000, 784,  28125 },
    { 48000,  810000, 512,  16875 },
    { 64000,  810000, 2048, 50625 },
    { 88200,  810000, 1568, 28125 },
    { 96000,  810000, 1024, 16875 },
    { 128000, 810000, 4096, 50625 },
    { 176400, 810000, 3136, 28125 },
    { 192000, 810000, 2048, 16875 },
};
// clang-format on

/**
 * @name: const FDcDpAudioMN *FDpAudioGetMN
 * @msg:  get the m and n value with the link_rate
 * @param {FDcDpAudioConfig} *audio_config is the audio config of dp
 * @param {u32} link_rate is the dp link rate
 * @return M and N
 */
const FDcDpAudioMN *FDpAudioGetMN(FDpCtrl *instance, u32 link_rate)
{
    FASSERT(instance != NULL);
    for (int i = 0; i < MAX_M_N; i++)
    {
        if ((instance->audio_config.sample_rate == DpAudioMN[i].sample_rate) &&
            (link_rate == DpAudioMN[i].link_rate))

        {
            return &DpAudioMN[i];
        }
    }

    return NULL;
}

/**
 * @name: FDpAudioSetPara
 * @msg:  set the audio config
 * @param {FDpCtrl*} instance is the dp config
 * @param {FDcDpAudioConfig *} audio_cfg
 * @return FDP_SUCCESS
 */
FError FDpAudioSetPara(FDpCtrl *instance)
{
    FASSERT(instance != NULL);

    u32 sec_window, hblank_period, period_clock, link_rate;
    u32 fs, ws, fs_accurc;
    const FDcDpAudioMN *m_n = NULL;
    u32 reg;
    uintptr address = instance->config.dp_channe_base_addr;
    FDpChannelRegWrite(address, PHYTIUM_DP_AUDIO_SEC_INPUT_SOURCE, 0x0);
    // FDpChannelRegWrite(address, FDP_AUDIO_DIRECT_CLKDIV, APB_CLK / audio_cfg->sample_rate);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUDIO_CHANNEL_COUNT, 2);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUDIO_CHANNEL_MAP, CHANNEL_DEFAULT);
    hblank_period = instance->timming.crtc_htotal - instance->timming.crtc_hdisplay;
    period_clock = instance->trans_config.pixel_clock;
    link_rate = instance->trans_config.pixel_clock * 1000;
    sec_window = 90 * hblank_period * link_rate / 100 / period_clock / 4;
    FDpChannelRegWrite(address, PHYTIUM_DP_SEC_DATA_WINDOW, sec_window);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUDIO_CATEGORY_CODE, 0xb5); /*IEC60958-3标准数字音频信号的设备类型*/
    reg = FDpChannelRegRead(address, PHYTIUM_DP_AUDIO_CLOCK_MODE);
    reg |= PHYTIUM_DP_AUDIO_CLOCK_MODE_MASK;
    FDpChannelRegWrite(address, PHYTIUM_DP_AUDIO_CLOCK_MODE, reg); /*时钟同步模式*/
    FDpChannelRegWrite(address, PHYTIUM_DP_AUDIO_CS_SOURCE_FORMAT, 0x0);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUDIO_INFOFRAME_SELECT,
                       AUDIO_INFOFRAME_AUDIO_DESCRIPTION); /*写入内部SRAM中 infoframe sdp 类型，选择audio*/

    switch (instance->audio_config.sample_rate)
    {
        case 32000:
            fs = ORIG_FREQ_32000;
            fs_accurc = SAMPLING_FREQ_32000;
            break;
        case 44100:
            fs = ORIG_FREQ_44100;
            fs_accurc = SAMPLING_FREQ_44100;
            break;
        case 48000:
            fs = ORIG_FREQ_48000;
            fs_accurc = SAMPLING_FREQ_48000;
            break;
        case 96000:
            fs = ORIG_FREQ_96000;
            fs_accurc = SAMPLING_FREQ_96000;
            break;
        case 176400:
            fs = ORIG_FREQ_176400;
            fs_accurc = SAMPLING_FREQ_176400;
            break;
        case 192000:
            fs = ORIG_FREQ_192000;
            fs_accurc = SAMPLING_FREQ_192000;
            break;
        default:
            fs = ORIG_FREQ_32000;
            fs_accurc = SAMPLING_FREQ_32000;
            break;
    }
    switch (instance->audio_config.sample_width)
    {
        case 16:
            ws = WORD_LENGTH_16;
            break;
        case 18:
            ws = WORD_LENGTH_18;
            break;
        case 20:
            ws = WORD_LENGTH_20;
            break;
        case 24:
            ws = WORD_LENGTH_24;
            break;
        default:
            ws = WORD_LENGTH_16;
            break;
    }
    FDpChannelRegWrite(address, PHYTIUM_DP_AUDIO_CS_LENGTH_ORIG_FREQ,
                       ((fs & ORIG_FREQ_MASK) << ORIG_FREQ_SHIFT) |
                           ((ws & WORD_LENGTH_MASK) << WORD_LENGTH_SHIFT));
    FDpChannelRegWrite(address, PHYTIUM_DP_AUDIO_CS_FREQ_CLOCK_ACCURACY,
                       (fs_accurc & SAMPLING_FREQ_MASK) << SAMPLING_FREQ_SHIFT);

    m_n = FDpAudioGetMN(instance, link_rate);

    if (m_n == NULL)
    {
        FDpChannelRegWrite(address, PHYTIUM_DP_AUDIO_MAUD, 0x0);
        FDpChannelRegWrite(address, PHYTIUM_DP_AUDIO_NAUD, 0x0);
    }
    else
    {
        FDpChannelRegWrite(address, PHYTIUM_DP_AUDIO_MAUD, m_n->m);
        FDpChannelRegWrite(address, PHYTIUM_DP_AUDIO_NAUD, m_n->n);
    }
    return FDP_SUCCESS;
}

/**
 * @name: FDpAudioEnable
 * @msg:  enable the audio
 * @param {FDpCtrl*} instance is the dp config
 * @return FMEDIA_DP_SUCCESS
 */
FError FDpAudioEnable(FDpCtrl *instance)
{
    FASSERT(instance != NULL);
    u32 reg;
    uintptr address = instance->config.dp_channe_base_addr;
    FDriverUdelay(500);

    reg = FDpChannelRegRead(address, PHYTIUM_DP_AUDIO_ENABLE);
    reg |= PHYTIUM_DP_AUDIO_ENABLE_MASK;
    reg &= ~PHYTIUM_DP_AUDIO_ENABLE_MUTE;
    FDpChannelRegWrite(address, PHYTIUM_DP_AUDIO_ENABLE, reg);

    return FDP_SUCCESS;
}
