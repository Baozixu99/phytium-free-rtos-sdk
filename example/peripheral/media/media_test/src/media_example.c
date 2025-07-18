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
 * FilePath: media_example.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2022-07-07 10:25:48
 * Description:  This file is for testing light the screen
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  Wangzq     2022/12/20  Modify the format and establish the version
 * 1.1  Wangzq     2023/07/07  change the third-party and driver relation
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
#include "media_example.h"
#include "media_example.h"
#include "fdcdp_crtc.h"

#include "media_common.h"
/************************** Variable Definitions *****************************/
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
/***************** Macros (Inline Functions) Definitions *********************/
static TaskHandle_t init_task;
static TaskHandle_t hpd_task;
static FFreeRTOSMedia os_media;
static GraphicsTest blt_buffer;

/************************** Function Prototypes ******************************/

/**
 * @name: BltVideoToFill
 * @msg:  write the rgb to the dc
 * @param {FDcCtrl} *instance_p is the struct of dc
 * @param {uintptr} offset is the addr
 * @param {u32} length is the length of the pixel
 * @param {void*} config is rgb value
 * @return Null
 */
static void PhyFramebufferWrite(FDcCtrl *instance_p, uintptr offset, u32 length,  void *config)
{
    u32 Index;
    for (Index = 0; Index < length; Index++)
    {
        FtOut32(instance_p->fb_virtual + offset + Index * 4, *((u32 *)(config + Index * 4)));
    }
}

/**
 * @name: BltVideoToFill
 * @msg:  fill the rgb into the dc
 * @param {FDcCtrl} *instance_p is the struct of dc
 * @param {GraphicsTest} config is the RGB value
 * @param {u32} width is the width of screen
 * @return Null
 */
static void BltVideoToFill(FDcCtrl *instance_p, GraphicsTest *config, u32 width, u32 height)
{
    FASSERT(instance_p  != NULL);
    FASSERT(config  != NULL);

    u32  stride;
    u32  blt;

    stride = FDcWidthToStride(width, 32, 1);
    memcpy(&blt, config, sizeof(GraphicsTest));
    for (int i = 0; i < (height); i++)
    {
        for (int j = 0; j < (width * 2); j++)
        {
            PhyFramebufferWrite(instance_p, i * stride + j * 4, 1, &blt);
        }
    }

}

/**
 * @name: FMediaDisplayDemo
 * @msg:  the demo for testing the media
 * @return Null
 */
FError FMediaDisplayDemo(void)
{
    u32 index;
    for (index = 0; index < FDP_INSTANCE_NUM; index ++)
    {   
        if (channel_mask & BIT(index))
        {
            blt_buffer.Red = 0x0;
            blt_buffer.Green = 0xff;
            blt_buffer.Blue = 0xff;
            blt_buffer.reserve = 0;
            BltVideoToFill(&os_media.dcdp_ctrl.dc_instance_p[index], &blt_buffer, 800, 600);
        }
    }
}

/* create media test*/
BaseType_t FFreeRTOSMediaCreate(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    /* enter critical region */
    taskENTER_CRITICAL();
    /* Media init task */
    xReturn = xTaskCreate((TaskFunction_t)FMediaInitTask, /* 任务入口函数 */
                          (const char *)"FMediaInitTask", /* 任务名字 */
                          1024,                         /* 任务栈大小 */
                          &os_media,                    /* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 2,                       /* 任务的优先级 */
                          (TaskHandle_t *)&init_task);

    /* HPD任务控制 */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSMediaHpdTask, /* 任务入口函数 */
                          (const char *)"FFreeRTOSMediaHpdTask", /* 任务名字 */
                          1024,                        /* 任务栈大小 */
                          &os_media,                   /* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1,                 /* 任务的优先级 */
                          (TaskHandle_t *)&hpd_task);
    /* exit critical region */
    taskEXIT_CRITICAL();
    return xReturn;
}

/* create media test*/
BaseType_t FFreeRTOSMediaDeinit()
{

    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    /* enter critical region */
    taskENTER_CRITICAL();
    /* Media init task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSMediaChannelDeinit, /* 任务入口函数 */
                          (const char *)"FMediaDeInitTask", /* 任务名字 */
                          1024,                         /* 任务栈大小 */
                          &os_media,                    /* 任务入口函数参数 */

                          (UBaseType_t)configMAX_PRIORITIES - 2,                       /* 任务的优先级 */
                          (TaskHandle_t *)&init_task);
                              /* exit critical region */
    taskEXIT_CRITICAL();
    return xReturn;
}