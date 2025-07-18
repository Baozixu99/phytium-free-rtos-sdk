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
 * FilePath: media_common.c
 * Date: 2025-07-11 11:32:48
 * LastEditTime: 2025-07-11 13:25:48
 * Description:  This file is for testing light the screen
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  Wangzq     2025/07/11  change the third-party and driver relation
 */
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "task.h"
#include "ftypes.h"
#include "fassert.h"
#include "fdebug.h"
#include "fparameters_comm.h"
#include "finterrupt.h"
#include "fkernel.h"
#include "event_groups.h"
#include "fcpu_info.h"

#include "fmedia_os.h"
#include "fdcdp.h"
#include "fdp_hw.h"
#include "fdp.h"
#include "fdcdp_reg.h"
#include "fdcdp_crtc.h"
#include "media_common.h"
typedef struct
{
    u32 bit_depth;
    u32 bpc;
    u32 color_depth;
    u32 clock_mode;
    u32 color_rep;
    u32 width;
    u32 height;
    u32 multi_mode;
} FuserCfg;

static const FuserCfg user_cfg = {
    .bit_depth = 8,
    .bpc = 8,
    .color_depth = 32,
    .clock_mode = 1,
    .color_rep = 0,
    .width = 800,
    .height = 600,
    .multi_mode = 0/*0:clone, 1 :horz, 2:vert*/
};
/*通道组合*/
#define CHANNEL_NONE     0x0  /*不选择*/
#define CHANNEL_DP_0        0x1  /*通道0*/
#define CHANNEL_DP_1        0x2  /*通道1*/
#define CHANNEL_DP_2        0x4  /*通道2*/
#define CHANNEL_DP_0_1      (CHANNEL_DP_0 | CHANNEL_DP_1)    /*通道0 1*/
#define CHANNEL_DP_0_2      (CHANNEL_DP_0 | CHANNEL_DP_2)    /*通道0 2*/
#define CHANNEL_DP_1_2      (CHANNEL_DP_1 | CHANNEL_DP_2)    /*通道1 2*/
#define CHANNEL_DP_ALL      (CHANNEL_DP_0 | CHANNEL_DP_1 | CHANNEL_DP_2)  /*通道0 1 2*/

static u32 channel_mask = CHANNEL_DP_0_1;  /* 默认选择双通道*/
/************************** Variable Definitions *****************************/
#define FMEDIA_EVT_INTR(index)             BIT(index)
#define FMEDIA_CHANNEL_0                    0
#define FMEDIA_CHANNEL_1                    1
static  EventGroupHandle_t media_event = NULL;
static u8 static_frame_buffer_address[1920 * 1080 * 4] __attribute__((aligned(128))) = {0};/*Framebuffer 大小 (字节) = 屏幕宽度 (像素) × 屏幕高度 (像素) × 每像素位数 (bpp) / 8*/
/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @name: FMediaCtrlProbe
 * @msg:  set the media control
 * @return 
 */
static void FMediaCtrlProbe(FFreeRTOSMedia *os_media)
{
    u32 index;
    for (index = 0; index < FDC_INSTANCE_NUM; index ++)
    {   if (channel_mask & BIT(index))
        {
            FDcDpCfgInitialize(&os_media->dcdp_ctrl, index);
            os_media->dcdp_ctrl.dc_instance_p[index].config = *FDcLookupConfig(index);
        }
    }
    for (index = 0; index < FDP_INSTANCE_NUM; index ++)
    {
        if (channel_mask & BIT(index))
        {
             os_media->dcdp_ctrl.dp_instance_p[index].config = *FDpLookupConfig(index);
        }
    }
    return ;
}

static void FFreeRTOSMediaSendEvent(u32 evt_bits)
{
    FASSERT(media_event);

    BaseType_t x_result = pdFALSE;
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    /*set the irq event for the task*/
    x_result = xEventGroupSetBitsFromISR(media_event, evt_bits, &xhigher_priority_task_woken);
}


static boolean FFreeRTOSMediaClearEvent(EventGroupHandle_t pvEventGroup, const uint32_t ulBitsToClear)
{
    FASSERT(media_event);
    EventBits_t event;
    event = xEventGroupClearBits(pvEventGroup, ulBitsToClear);/*clear the intr bits*/
    return TRUE;
}

static boolean FFreeRTOSMediaWaitEvent(u32 evt_bits, TickType_t wait_delay)
{
    FASSERT(media_event);
    EventBits_t event;
    event = xEventGroupWaitBits(media_event, evt_bits,
                                pdTRUE, pdFALSE, wait_delay);/*wait the irq event for the task*/
    if (event & evt_bits)
    {
        return TRUE;
    }
    return FALSE;
}

/* create the hpd task */
void FFreeRTOSMediaHpdTask(FFreeRTOSMedia *os_media)
{
    u32 index;
    u32 ret = 0 ;
    FFreeRTOSMediaWaitEvent(FMEDIA_EVT_INTR(FMEDIA_CHANNEL_0) | FMEDIA_EVT_INTR(FMEDIA_CHANNEL_1), portMAX_DELAY);
    os_media->dcdp_ctrl.multi_mode = user_cfg.multi_mode;
    for (;;)
    {
        for (index = 0; index < FDP_INSTANCE_NUM; index++)
        {
            if (channel_mask & BIT(index))
            {
            if (os_media->dcdp_ctrl.connect_flg[index] == FDCDP_DISCONNCET_TO_CONNECT)
            {
                FDcDpCfgInitialize(&os_media->dcdp_ctrl, index);
                ret = FDcDpInitial(&os_media->dcdp_ctrl, index, user_cfg.width, user_cfg.height);
                FFreeRTOSMediaClearEvent(media_event, FMEDIA_EVT_INTR(index));
                if (ret == FDP_SUCCESS)
                {
                    printf("Hpd task finish , reinit the dp success.\r\n");
                }
                os_media->dcdp_ctrl.connect_flg[index] = FDCDP_IS_INITIALIZED;
            }
            }
        vTaskDelay(200);
    }
        vTaskDelete(NULL);
}
}

/**
 * @name: FDcDpIrqAllEnable
 * @msg:  enable the irq
 * @param  {FDcDp} *instance_p is the instance of dcdp
 * @return Null
 */
static void FDcDpIrqAllEnable(FDcDp *instance_p)
{
    int index = 0;
    FDcDpIntrEventType event_type = FDCDP_HPD_IRQ_CONNECTED;
    for (index = 0; index < FDP_INSTANCE_NUM; index++)
    {
        for (event_type = 0; event_type < FDCDP_INTR_MAX_NUM; event_type++)
        {
            FDcDpIrqEnable(instance_p, index, event_type);
        }
    }
}

/**
 * @name: FFreeRTOSMediaHpdConnectCallback
 * @msg:  the hpd connect event
 * @param  {void} *args is the instance of dcdp
 * @param  {u32} index is the channel
 * @return Null
 */
static void FFreeRTOSMediaHpdConnectCallback(void *args, u32 index)
{
    FASSERT(args != NULL);
    FDcDp *instance_p = (FDcDp *)args;
    FDpChannelRegRead(instance_p->dp_instance_p[index].config.dp_channe_base_addr, PHYTIUM_DP_INTERRUPT_STATUS); /*clear interrupt*/
    FFreeRTOSMediaSendEvent(FMEDIA_EVT_INTR(index));
    instance_p->connect_flg[index] = FDCDP_DISCONNCET_TO_CONNECT;
    printf("Dp:%d connect\r\n", index);
}

/**
 * @name: FFreeRTOSMediaHpdBreakCallback
 * @msg:  the hpd disconnect event
 * @param  {void} *args is the instance of dcdp
 * @param  {u32} index is the channel
 * @return Null
 */
static void FFreeRTOSMediaHpdBreakCallback(void *args, u32 index)
{
    FASSERT(args != NULL);
    FDcDp *instance_p = (FDcDp *)args;
    FDpChannelRegRead(instance_p->dp_instance_p[index].config.dp_channe_base_addr, PHYTIUM_DP_INTERRUPT_STATUS); /*clear interrupt*/
    instance_p->connect_flg[index] = FDCDP_CONNECT_TO_DISCONNCET;
    FFreeRTOSMediaSendEvent(FMEDIA_EVT_INTR(index));
    printf("Dp:%d disconnect\r\n", index);
}

/**
 * @name: FFreeRTOSMediaAuxTimeoutCallback
 * @msg:  the aux timeout  event
 * @param  {void} *args is the instance of dcdp
 * @param  {u32} index is the channel
 * @return Null
 */
static void FFreeRTOSMediaAuxTimeoutCallback(void *args, u32 index)
{
    FASSERT(args != NULL);
    FDcDp *instance_p = (FDcDp *)args;
    FDpChannelRegRead(instance_p->dp_instance_p[index].config.dp_channe_base_addr, PHYTIUM_DP_INTERRUPT_STATUS); /*clear interrupt*/
    printf("Dp:%d aux connect timeout\r\n", index);
}

/**
 * @name: FFreeRTOSMediaAuxErrorCallback
 * @msg:  the aux error  event
 * @param  {void} *args is the instance of dcdp
 * @param  {u32} index is the channel
 * @return Null
 */
static void FFreeRTOSMediaAuxErrorCallback(void *args, u32 index)
{
    FASSERT(args != NULL);
    FDcDp *instance_p = (FDcDp *)args;
    FDpChannelRegRead(instance_p->dp_instance_p[index].config.dp_channe_base_addr, PHYTIUM_DP_INTERRUPT_STATUS); /*clear interrupt*/
    printf("Dp:%d aux connect error\r\n", index);
}

/**
 * @name: FFreeRTOSMediaIrqSet
 * @msg:  set the irq event and instance
 * @param {FDcDp} *instance_p is the instance of dcdp
 * @return Null
 */
static void FFreeRTOSMediaIrqSet(FDcDp *instance_p)
{
    FASSERT(instance_p != NULL);
    u32 cpu_id;
    u32 index;

    GetCpuId(&cpu_id);
    for (index = 0; index < FDC_INSTANCE_NUM; index ++)
    {
        if (channel_mask & BIT(index))
        {
            InterruptSetTargetCpus(instance_p->dp_instance_p[index].config.irq_num, cpu_id);/*the dc0 and dc1 have the same num of irq_num*/
            InterruptSetPriority(instance_p->dp_instance_p[index].config.irq_num, FREERTOS_MEDIA_IRQ_PRIORITY);/*dp0 and dp1 have the same irq_num*/
            InterruptInstall(instance_p->dp_instance_p[index].config.irq_num, FDcDpInterruptHandler, instance_p, "");
            InterruptUmask(instance_p->dp_instance_p[index].config.irq_num);
        }
    }

    FDcDpRegisterHandler(instance_p, FDCDP_HPD_IRQ_CONNECTED, FFreeRTOSMediaHpdConnectCallback, (void *)instance_p);
    FDcDpRegisterHandler(instance_p, FDCDP_HPD_IRQ_DISCONNECTED, FFreeRTOSMediaHpdBreakCallback, (void *)instance_p);
    FDcDpRegisterHandler(instance_p, FDCDP_AUX_REPLY_TIMEOUT, FFreeRTOSMediaAuxTimeoutCallback, (void *)instance_p);
    FDcDpRegisterHandler(instance_p, FDCDP_AUX_REPLY_ERROR, FFreeRTOSMediaAuxErrorCallback, (void *)instance_p);
}

/**
 * @name: FMediaInitTask
 * @msg:  init the media control
 * @return ret,FMEDIA_DP_SUCCESS means success
 */
FError FMediaInitTask(FFreeRTOSMedia *os_media)
{
    FASSERT_MSG(NULL == media_event, "Event group exists.");
    FASSERT_MSG((media_event = xEventGroupCreate()) != NULL, "Create event group failed.");
    u32 index;
    FError ret = FT_SUCCESS;
    /*设置用户参数*/
    FMediaCtrlProbe(os_media);
    for (index = 0; index < FDP_INSTANCE_NUM; index ++)
    {
        if (channel_mask & BIT(index))
        {
        os_media->dcdp_ctrl.dp_instance_p[index].trans_config.bit_depth = user_cfg.bit_depth;

        os_media->dcdp_ctrl.dc_instance_p[index].crtc.bpc = user_cfg.bpc;

        os_media->dcdp_ctrl.dc_instance_p[index].color_depth = user_cfg.color_depth;

        os_media->dcdp_ctrl.dp_instance_p[index].trans_config.clock_mode = user_cfg.clock_mode;

        os_media->dcdp_ctrl.dp_instance_p[index].trans_config.color_rep_format = user_cfg.color_rep;

        os_media->dcdp_ctrl.dc_instance_p[index].channel = index;

        os_media->dcdp_ctrl.dc_instance_p[index].fb_addr = (uintptr)static_frame_buffer_address ;/*当前例程虚拟地址和物理地址一致，实际需要根据需要进行映射*/

        os_media->dcdp_ctrl.dc_instance_p[index].fb_virtual = (uintptr)static_frame_buffer_address ;/*当前例程虚拟地址和物理地址一致，实际需要根据需要进行映射*/

        os_media->dcdp_ctrl.dp_instance_p[index].port = index;
        
        FDcDpGeneralCfgInitial(&os_media->dcdp_ctrl, index);

        FFreeRTOSMediaHwInit(os_media, index,user_cfg.width, user_cfg.height);
        }
    }
    FFreeRTOSMediaIrqSet(&os_media->dcdp_ctrl);
    FDcDpIrqAllEnable(&os_media->dcdp_ctrl);
    vTaskDelete(NULL);
    return ret;
}
/**
 * @name: FFreeRTOSMediaChannelDeinit
 * @msg:  deinit the media
 * @param  {u32} id is the number of dcdp
 * @return Null
 */
void FFreeRTOSMediaChannelDeinit(FFreeRTOSMedia *os_media)
{
    taskENTER_CRITICAL();
    vEventGroupDelete(media_event);
    media_event = NULL;
    u32 index;
    for (index = 0; index < FDP_INSTANCE_NUM; index ++)
    {
        if (channel_mask & BIT(index))
        {
            FDcDpDeInitialize(&os_media->dcdp_ctrl, index);/*deinit the id channel*/
        }
    }
    taskEXIT_CRITICAL();
    return;
}
