/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: gdma_memcpy.c
 * Date: 2022-07-20 11:07:42
 * LastEditTime: 2022-07-20 11:16:57
 * Description:  This files is for GDMA task implementations 
 *
 * Modify History:
 *  Ver      Who           Date         Changes
 * -----    ------       --------      --------------------------------------
 *  1.0    zhugengyu     2022/7/27     init commit
 *  2.0    liqiaozhong   2023/11/10    synchronous update with standalone sdk
 *  3.0    liqiaozhong   2024/4/22     add no letter shell mode, adapt to auto-test system
 */

/***************************** Include Files *********************************/
#include <string.h>
#include <stdbool.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "fkernel.h"
#include "fassert.h"
#include "fdebug.h"

#include "fgdma_os.h"
/************************** Constant Definitions *****************************/
#define FGDMA_DEBUG_TAG "GDMA-MEM"
#define FGDMA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_WARN(format, ...)    FT_DEBUG_PRINT_W(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_INFO(format, ...)    FT_DEBUG_PRINT_I(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)

/* user-define */
#define FGDMA_CONTROLLER_ID        FGDMA0_ID
#define GDMA_CHANNEL_ID            0U
#define GDMA_TRANS_LEN             1024U
#define GDMA_ADDR_ALIGNMENT        16U
#define GDMA_MEMCPY_TASK_PRIORITY  3
#define TIMER_OUT                  (pdMS_TO_TICKS(4000UL))
/**************************** Type Definitions *******************************/
enum
{
    GDMA_EXAMPLE_SUCCESS        = 0,
    GDMA_EXAMPLE_UNKNOWN_STATE  = 1,
    GDMA_EXAMPLE_INIT_FAILURE   = 2,
    GDMA_EXAMPLE_MEMCPY_FAILURE = 3,              
};
/************************** Variable Definitions *****************************/
static FFreeRTOSGdma *gdma_instance_p = NULL;
static FFreeRTOSGdmaChanCfg os_channel_config;
static TaskHandle_t gdma_trans_task = NULL;
const UBaseType_t gdma_trans_task_index = 0; /* the index must set to 0 */
static QueueHandle_t xQueue = NULL;
static uint8_t src_data[GDMA_TRANS_LEN] __attribute__((aligned(GDMA_ADDR_ALIGNMENT))) = {0U}; /* should be aligned with both read and write burst size, defalut: 16-byte */
static uint8_t dst_data[GDMA_TRANS_LEN] __attribute__((aligned(GDMA_ADDR_ALIGNMENT))) = {0U};
/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Function Prototypes ****************************/

/*************************** Function Definitions ****************************/
static void GdmaMemcpyAckChanXEnd(uint32_t channel_id, void *args)
{
    FASSERT(channel_id < FFREERTOS_GDMA_NUM_OF_CHAN);
    BaseType_t xhigher_priority_task_woken = pdFALSE;

    FGDMA_INFO("FreeRTOS ack: GDMA channel-%d transfer end.", channel_id);

    vTaskNotifyGiveIndexedFromISR(gdma_trans_task, gdma_trans_task_index, &xhigher_priority_task_woken);
    gdma_trans_task = NULL; /* 将目标任务句柄清空，防止产生不必要的通知 */
    portYIELD_FROM_ISR(xhigher_priority_task_woken); /* 如果中断触发了更高优先级的任务，确保更高优先级的任务尽快执行 */
}

static FError GdmaInit(void)
{   
    FError err = FFREERTOS_GDMA_OK;

    gdma_instance_p = FFreeRTOSGdmaInit(FGDMA_CONTROLLER_ID);

    if (gdma_instance_p == NULL)
    {
        err = FFREERTOS_GDMA_COMMON_ERR;
    }

    FGDMA_INFO("%s return with err: %d", __func__, err);
    return err;
}

static FError GdmaMemcpy(void)
{
    FError err = FFREERTOS_GDMA_OK;
    uint32_t notify_result;
    const TickType_t max_block_time = pdMS_TO_TICKS(3000UL);

    /* os channel config set */
    os_channel_config.trans_mode = FFREERTOS_GDMA_OPER_DIRECT;
    os_channel_config.src_addr = (uintptr_t)src_data;
    os_channel_config.dst_addr = (uintptr_t)dst_data;
    os_channel_config.trans_length = GDMA_TRANS_LEN;

    err = FFreeRTOSGdmaChanConfigure(gdma_instance_p, GDMA_CHANNEL_ID, &os_channel_config);
    if (err != FFREERTOS_GDMA_OK)
    {
        FGDMA_ERROR("FFreeRTOSGdmaChanConfigure() failed.");
        goto memcpy_exit;
    }

    FFreeRTOSGdmaChanRegisterEvtHandler(gdma_instance_p, 
                                        GDMA_CHANNEL_ID, 
                                        FFREERTOS_GDMA_CHAN_EVT_TRANS_END,
                                        GdmaMemcpyAckChanXEnd,
                                        NULL);

    memset((void *)src_data, 'A', GDMA_TRANS_LEN);
    memset((void *)dst_data, 0, GDMA_TRANS_LEN);

    gdma_trans_task = xTaskGetCurrentTaskHandle(); /* store the handle of the calling task. */

    FFreeRTOSGdmaChanStart(gdma_instance_p, GDMA_CHANNEL_ID);

    notify_result = ulTaskNotifyTakeIndexed(gdma_trans_task_index, pdTRUE, max_block_time); /* wait to be notified that the transmission is complete */
    if (notify_result != 1)
    {
        err = FFREERTOS_GDMA_SEMA_ERR;
        FGDMA_ERROR("Wait GDMA finish notify_result timeout.");
        goto memcpy_exit;
    }

    /* compare if memcpy success */
    if (0 == memcmp(src_data, dst_data, GDMA_TRANS_LEN))
    {
        taskENTER_CRITICAL();
        printf("src buf...\r\n");
        FtDumpHexByte((const uint8_t *)src_data, min((size_t)GDMA_TRANS_LEN, (size_t)64U));
        printf("dst buf...\r\n");
        FtDumpHexByte((const uint8_t *)dst_data, min((size_t)GDMA_TRANS_LEN, (size_t)64U));
        taskEXIT_CRITICAL(); 
    }
    else
    {
        err = FFREERTOS_GDMA_COMMON_ERR;
        FGDMA_ERROR("src != dst, GDMA memcpy failed.");
        goto memcpy_exit;
    }

memcpy_exit:
    if (FFreeRTOSGdmaChanStop(gdma_instance_p, GDMA_CHANNEL_ID) != FFREERTOS_GDMA_OK)
    {
        FGDMA_ERROR("FFreeRTOSGdmaChanStop() failed.");
        err = FFREERTOS_GDMA_COMMON_ERR;
    }

    if (FFreeRTOSGdmaChanDeconfigure(gdma_instance_p, GDMA_CHANNEL_ID) != FFREERTOS_GDMA_OK)
    {
        FGDMA_ERROR("FFreeRTOSGdmaChanDeconfigure() failed.");
        err = FFREERTOS_GDMA_COMMON_ERR;
    }

    FGDMA_INFO("%s return with err: %d", __func__, err);
    return err;
}

static void GdmaMemcpyTask()
{
    FError err = FFREERTOS_GDMA_OK;
    int task_res = GDMA_EXAMPLE_SUCCESS;

    err = GdmaInit();
    if (err != FFREERTOS_GDMA_OK)
    {
        task_res = GDMA_EXAMPLE_INIT_FAILURE;
        FGDMA_ERROR("GdmaInit() failed.");
        goto task_exit;
    }

    err = GdmaMemcpy();
    if (err != FFREERTOS_GDMA_OK)
    {
        task_res = GDMA_EXAMPLE_MEMCPY_FAILURE;
        FGDMA_ERROR("GdmaMemcpy() failed.");
        goto task_exit;
    }

task_exit:
    xQueueSend(xQueue, &task_res, 0);

    vTaskDelete(NULL);
}

int FFreeRTOSGdmaMemcpy(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = GDMA_EXAMPLE_UNKNOWN_STATE;

    xQueue = xQueueCreate(1, sizeof(int)); /* 创建消息队列 */
    if (xQueue == NULL)
    {
        FGDMA_ERROR("xQueue create failed.");
        goto exit;
    }

    xReturn = xTaskCreate((TaskFunction_t)GdmaMemcpyTask,         /* 任务入口函数 */
                          (const char *)"GdmaMemcpyTask",         /* 任务名字 */
                          (uint16_t)4096,                         /* 任务栈大小 */
                          NULL,                                   /* 任务入口函数参数 */
                          (UBaseType_t)GDMA_MEMCPY_TASK_PRIORITY, /* 任务优先级 */
                          NULL);                                  /* 任务句柄 */
    if (xReturn == pdFAIL)
    {
        FGDMA_ERROR("xTaskCreate GdmaMemcpyTask failed.");
        goto exit;
    }

    xReturn = xQueueReceive(xQueue, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        FGDMA_ERROR("xQueue receive timeout.");
        goto exit;
    }

exit:
    if (xQueue != NULL)
    {
        vQueueDelete(xQueue);
    }

    if (task_res != GDMA_EXAMPLE_SUCCESS)
    {
        printf("%s@%d: GDMA memcpy example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return task_res;
    }
    else
    {
        printf("%s@%d: GDMA memcpy example [success].\r\n", __func__, __LINE__);
        return task_res;
    }
}