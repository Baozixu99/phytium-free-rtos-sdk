/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fdcdp.c
 * Date: 2022-09-09 09:23:30
 * LastEditTime: 2022/12/20 10:10:31
 * Description:  This file is for connecting the dc and dp
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2022/12/20  Modify the format and establish the version
 */
#include <string.h>
#include "fio.h"
#include "fdrivers_port.h"
#include "fassert.h"
#include "ftypes.h"
#include "fdrivers_port.h"
#include "fparameters_comm.h"

#include "fdcdp.h"
#include "fdc.h"
#include "fdp.h"
#include "fdc_hw.h"
#include "fdp_hw.h"
#include "fdcdp_aux.h"
#include "fdcdp_crtc.h"
#include "fdcdp_reg.h"
#include "fdcdp_edid.h"
#include "fdcdp_audio.h"

const FDcTimming DcSyncTable[FDC_GOP_MAX_MODENUM] = {
    {800, 640, 656, 752, 0, 525, 480, 490, 492, 0},         /*640 * 480 60Hz*/
    {1056, 800, 840, 968, 1, 628, 600, 601, 605, 1},        /*800*600p 60Hz*/
    {1344, 1024, 1048, 1184, 0, 806, 768, 771, 777, 0},     /*1024*768p 60Hz*/
    {1650, 1280, 1390, 1430, 1, 750, 720, 725, 730, 1},     /*1280*720p 60Hz*/
    {1792, 1366, 1436, 1579, 1, 798, 768, 771, 774, 1},     /*1366*768p_60Hz*/
    {2200, 1920, 2008, 2052, 1, 1125, 1080, 1084, 1089, 1}, /*1920*1080p 60Hz*/
    {2160, 1600, 1664, 1856, 1, 1250, 1200, 1201, 1204, 1}, /*1600*1200 60Hz*/
    {1680, 1280, 1352, 1480, 0, 831, 800, 803, 809, 1},     /*1280x800 60Hz*/
    {928, 800, 840, 888, 1, 525, 490, 493, 496, 1},         /*800x480 60Hz*/
    {1688, 1280, 1328, 1440, 1, 1066, 1024, 1025, 1028, 1}, /*1280*768 60Hz*/
    {1664, 1280, 1344, 1472, 0, 798, 768, 771, 778, 1}      /*1280x1024 60Hz*/
};

const FDpTimming DpSyncTable[FDC_GOP_MAX_MODENUM] = {
    {800, 640, 96, 144, 1, 525, 480, 2, 35, 0},      /*640*480 60Hz*/
    {1056, 800, 128, 216, 0, 628, 600, 4, 27, 0},    /*800*600p 60Hz*/
    {1344, 1024, 136, 296, 1, 806, 768, 6, 35, 1},   /*1024*768p 60Hz*/
    {1650, 1280, 40, 260, 1, 750, 720, 5, 25, 1},    /*1280*720 60Hz*/
    {1792, 1366, 143, 356, 0, 798, 768, 3, 27, 0},   /*1366*768p_60Hz*/
    {2200, 1920, 44, 192, 0, 1125, 1080, 5, 41, 0},  /*1920*1080p 60Hz*/
    {2160, 1600, 192, 496, 0, 1250, 1200, 3, 49, 0}, /*1600x1200 60Hz*/
    {1680, 1280, 128, 328, 1, 831, 800, 6, 28, 0},   /*1280x800 60Hz*/
    {928, 800, 48, 88, 0, 528, 480, 3, 32, 0},       /*800x480 60Hz*/
    {1664, 1280, 128, 320, 0, 798, 768, 7, 27, 1},   /*1280*768 60Hz*/
    {1688, 1280, 112, 360, 0, 1066, 1024, 3, 41, 0}  /*1280x1024 60Hz*/
};

#define FDCDP_DEBUG_TAG "FDCDP"
#define FDCDP_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FDCDP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDCDP_INFO(format, ...) FT_DEBUG_PRINT_I(FDCDP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDCDP_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FDCDP_DEBUG_TAG, format, ##__VA_ARGS__)


/************************** Function Prototypes ******************************/

/**
 * @name: FDcDpCfgInitialize
 * @msg:  init the FDcDp instance config
 * @param {FDcDp} *instance is the media parameter of display
 * @param {u32} index is the index of display
 * @return :FDP_SUCCESS:success;others:failed
 */
FError FDcDpCfgInitialize(FDcDp *instance, u32 index)
{
    FASSERT(instance != NULL);
    FError ret = FDP_SUCCESS;
    if (FT_COMPONENT_IS_READY == instance->is_ready[index])
    {
        FDCDP_DEBUG("Device is %d already initialized.\r\n", index);
        return ret;
    }
    instance->is_ready[index] = FT_COMPONENT_IS_READY;

    return ret;
}

/**
 * @name: FDcDpSetTiming
 * @msg:  set the FDcDp timing config
 * @param {FDcDp} *instance is the media parameter of display
 * @param {u32} channel is the index of display
 * @return 
 */
void FDcDpSetTiming(FDcDp *instance, u32 channel)
{
    u32 pixel_id = instance->dc_instance_p[channel].timing_id;

    memcpy(&instance->dc_instance_p[channel].timming, &DcSyncTable[pixel_id],
           sizeof(DcSyncTable[pixel_id]));

    memcpy(&instance->dp_instance_p[channel].timming, &DpSyncTable[pixel_id],
           sizeof(DpSyncTable[pixel_id]));
}

/**
 * @name: FDcDpGeneralCfgInitial
 * @msg:  set the FDcDp config
 * @param {FDcDp} *instance is the media parameter of display
 * @param {u8} channel is the index of display
 * @return 
 */
void FDcDpGeneralCfgInitial(FDcDp *instance, u8 channel)
{
    /*phy soft reset*/
    FDpLinkPhyInit(&instance->dp_instance_p[channel], 810);

    FDcHwAhbReset(&instance->dc_instance_p[channel], channel, FDC_RESET_AHB); /*reset the ahb*/

    FDcHwAhbReset(&instance->dc_instance_p[channel], channel, FDC_RESET_CORE);
}

/**
 * @name: FDcDpInitial
 * @msg:  init the DcDp
 * @param {FDcDp} *instance is the DcDp parameter of display
 * @param {u8} channel is the channel of display
 * @param {u32} width is the width of display
 * @param {u32} height is the height of display
 * @return :FDP_SUCCESS:success;others:failed
 */
FError FDcDpInitial(FDcDp *instance, u8 channel, u32 width, u32 height)
{
    FError ret = FDP_SUCCESS;
    FASSERT(instance->is_ready[channel] == FT_COMPONENT_IS_READY);
    u8 edid_buffer[256];
    if (instance->is_initialized[channel] == FDCDP_IS_INITIALIZED)
    {
        FDCDP_DEBUG("Device %d is already initialized. \r\n", channel);
        return ret;
    }

    instance->dc_instance_p[channel].timing_id = FDcResToModeNum(&instance->dc_instance_p[channel],
                                                                 width, height);

    instance->dp_instance_p[channel]
        .trans_config.pixel_clock = instance->dc_instance_p[channel].pixel_clock =
        FDcResToPixelClock(instance->dc_instance_p[channel].timing_id);

    FDcDpSetTiming(instance, channel);

    FDpInitAux(&instance->dp_instance_p[channel]);

    memset(edid_buffer, 0, sizeof(edid_buffer));
    ret = FDpGetEdid(&instance->dp_instance_p[channel], edid_buffer);
    if (ret != FDP_SUCCESS)
    {
        FDCDP_ERROR("FDCDP GetEdid failed");
        return ret;
    }
    ret = FDpStartLinkTraining(&instance->dp_instance_p[channel]);

    if (ret != FDP_SUCCESS)
    {
        FDCDP_ERROR("FDCDP TRAINING failed");
        return ret;
    }
    /*res*/
    FDpHwScramblerReset(&instance->dp_instance_p[channel]);

    ret = FDcHwConfigPixelClock(&instance->dc_instance_p[channel],
                                instance->dc_instance_p[channel].pixel_clock);
    if (ret != FDP_SUCCESS)
    {
        FDCDP_ERROR("FDcHwConfigPixelClock failed");
        return ret;
    }

    ret = FDpSetCrtcEnable(&instance->dp_instance_p[channel]);
    if (ret != FDP_SUCCESS)
    {
        FDCDP_ERROR("FDpSetCrtcEnable failed");
        return ret;
    }

    FDpHwEnableOutput(&instance->dp_instance_p[channel]);

    ret = FDcModeChangeConfigBegin(&instance->dc_instance_p[channel]);
    if (ret != FDP_SUCCESS)
    {
        FDCDP_ERROR("FDcModeChangeConfigBegin failed");
        return ret;
    }
    FDcHwSetFbAddr(&instance->dc_instance_p[channel]);

    ret = FDcSetCrtcEnable(&instance->dc_instance_p[channel]);
    if (ret != FDP_SUCCESS)
    {
        FDCDP_ERROR("FDcSetCrtcEnable failed");
        return ret;
    }
    FDpAudioSetPara(&instance->dp_instance_p[channel]);
    FDpAudioEnable(&instance->dp_instance_p[channel]);
    instance->is_initialized[channel] = FDCDP_IS_INITIALIZED;
    instance->has_train_dp[channel] = FDCDP_IS_INITIALIZED;
    FDCDP_DEBUG("FDcDpInitial success,channel:%d", channel);
    return ret;
}

/*
 * @name: FDcDpDeInitialize
 * @msg:  deinit the DcDp
 * @param {FDcDp} *instance_p is the DcDp parameter of display
 * @param {u32} index is the dcdp channel of display
 * @return FDP_SUCCESS
 */
FError FDcDpDeInitialize(FDcDp *instance, u32 index)
{
    FASSERT(instance);
    FError ret = FDP_SUCCESS;

    instance->dc_instance_p[index].channel = index;
    instance->dp_instance_p[index].channel = index;
    /* disable dc  */
    FDcHwReset(&instance->dc_instance_p[index]);
    /* disable dp */
    FDpHwReset(&instance->dp_instance_p[index]);

    instance->is_ready[index] = 0;
    instance->is_initialized[index] = FDCDP_NOT_INITIALIZED;
    return ret;
}

/*
 * @name: FDcDpHotPlug
 * @msg:  the hot plug information
 * @param {FDcDp} *instance_p is the DcDp parameter of display
 * @param {u32} index is the dp channel of display
 * @param {FDcDpConnectStatus} connect_status is the dp connect status
 * @return
 */
void FDcDpHotPlug(FDcDp *instance_p, u32 index, FDcDpConnectStatus connect_status)
{
    FASSERT(instance_p);
    switch (connect_status)
    {
        case FDCDP_CONNECT_TO_DISCONNCET:
            FDCDP_DEBUG("The channel:%d  has been disconnected,please check it", index);
            break;
        case FDCDP_DISCONNCET_TO_CONNECT:
            FDCDP_DEBUG("The channel:%d  has been connected", index);
        default:
            break;
    }
}