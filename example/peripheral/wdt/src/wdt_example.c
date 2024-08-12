/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: wdt_example.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2022-07-11 11:32:48
 * Description:  This file is for wdt test example functions.
 *
 * Modify History:
 *  Ver      Who            Date           Changes
 * -----   ------         --------     --------------------------------------
 *  1.0   wangxiaodong    2022/8/9      first release
 *  2.0   liqiaozhong     2024/4/22     add no letter shell mode, adapt to auto-test system
 */

#include <string.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "fparameters.h"
#include "fgeneric_timer.h"
#include "fdebug.h"

#include "fwdt_os.h"
#include "fcpu_info.h"

#define FWDT_DEBUG_TAG "WDT-EXAMPLE"
#define FWDT_ERROR(format, ...)   FT_DEBUG_PRINT_E(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWDT_WARN(format, ...)    FT_DEBUG_PRINT_W(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWDT_INFO(format, ...)    FT_DEBUG_PRINT_I(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWDT_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)

/* user-define */
#define WDT_CONTROLLER_ID           FWDT0_ID
#define WDT_FEED_TIMES              3 /* 喂狗操作次数 */  
#define WDT_ONCE_TIMEOUT            3 /* watchdog timeout value in seconds */
#define WDT_FEED_PERIOD             (pdMS_TO_TICKS(15000UL)) /* 喂狗操作最长执行时间，设置时需要考虑WDT_FEED_TIMES与WDT_ONCE_TIMEOUT */
#define WDT_FEED_TASK_PRIORITY      3
#define WDT_EXAMPLE_TIMEOUT         (WDT_FEED_PERIOD + (pdMS_TO_TICKS(3000UL))) /* 用例最长执行时间 */

enum
{
    WDT_EXAMPLE_SUCCESS = 0,
    WDT_EXAMPLE_UNKNOWN_STATE,
    WDT_EXAMPLE_INIT_FAILURE,
    WDT_EXAMPLE_FEED_FAILURE,             
};

static FFreeRTOSWdt *os_wdt_ctrl_p;
static u32 wdt_feed_count = 0; /* 记录喂狗次数 */
static QueueHandle_t xQueue = NULL;
static TaskHandle_t wdt_feed_task = NULL;
const UBaseType_t wdt_feed_task_index = 0; /* the index must set to 0 */

static void FFreeRTOSWdtInterruptHandler(s32 vector, void *param)
{
    FASSERT(param != NULL);
    FWdtCtrl *wdt_ctrl_p_irq = (FWdtCtrl *)param;
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    u32 second = 0;

    if (wdt_feed_count >= WDT_FEED_TIMES)
    {
        FWdtRefresh(wdt_ctrl_p_irq);
        vTaskNotifyGiveIndexedFromISR(wdt_feed_task, wdt_feed_task_index, &xhigher_priority_task_woken);
        printf("Receive WDT timeout intr and try to return.\r\n");
        portYIELD_FROM_ISR(xhigher_priority_task_woken); /* 如果中断触发了更高优先级的任务，确保更高优先级的任务尽快执行 */
    }
    else
    {
        FWdtRefresh(wdt_ctrl_p_irq);
        second = GenericTimerRead(GENERIC_TIMER_ID0) / GenericTimerFrequecy();
        printf("Receive WDT timeout intr and refresh it, count: %d, second: %d\r\n", wdt_feed_count, second);
        wdt_feed_count ++;
    }
}

static FError WdtExampleInit(void)
{
    FError ret = FWDT_SUCCESS;
    
    /* init wdt controller */
    os_wdt_ctrl_p = FFreeRTOSWdtInit(WDT_CONTROLLER_ID);
    if (os_wdt_ctrl_p == NULL)
    {
        ret = FREERTOS_WDT_INIT_ERROR;
        FWDT_ERROR("FFreeRTOSWdtInit() error.");
        goto wdt_init_exit;
    }

    /* set wdt timeout interrupt handler */
    u32 cpu_id = 0;
    FWdtCtrl *pctrl = &os_wdt_ctrl_p->wdt_ctrl;
    FWdtConfig *pconfig = &os_wdt_ctrl_p->wdt_ctrl.config;
    GetCpuId(&cpu_id);
    printf("WDT_CONTROLLER_ID: %d, pconfig->irq_num: %ld\r\n", WDT_CONTROLLER_ID, pconfig->irq_num);
    InterruptSetTargetCpus(pconfig->irq_num, cpu_id);

    /* interrupt init */
    InterruptSetPriority(pconfig->irq_num, pconfig->irq_prority);
    InterruptInstall(pconfig->irq_num, FFreeRTOSWdtInterruptHandler, pctrl, pconfig->instance_name);
    InterruptUmask(pconfig->irq_num);

wdt_init_exit:
    FWDT_INFO("%s return with ret: %d", __func__, ret);
    return ret;
}

static FError WdtExampleFeed(void)
{
    FError ret = FWDT_SUCCESS;
    uint32_t notify_result;
    u32 wdt_timeout = WDT_ONCE_TIMEOUT; /* 为了传入地址，重新赋值 */
    const TickType_t max_block_time = WDT_FEED_PERIOD;

    /* set wdt timeout value */
    ret = FFreeRTOSWdtControl(os_wdt_ctrl_p, FREERTOS_WDT_CTRL_SET_TIMEOUT, &wdt_timeout);
    if (ret != FWDT_SUCCESS)
    {
        FWDT_ERROR("FFreeRTOSWdtControl SET_TIMEOUT error.");
        goto wdt_feed_exit;
    }

    wdt_feed_task = xTaskGetCurrentTaskHandle();

    /* start wdt controller */
    ret = FFreeRTOSWdtControl(os_wdt_ctrl_p, FREERTOS_WDT_CTRL_START, NULL);
    if (ret != FWDT_SUCCESS)
    {
        FWDT_ERROR("FFreeRTOSWdtControl CTRL_START error.");
        goto wdt_feed_exit;
    }

    notify_result = ulTaskNotifyTakeIndexed(wdt_feed_task_index, pdTRUE, max_block_time); /* 等待喂狗完毕后产生的超时中断 */
    if (notify_result != 1)
    {
        ret = FREERTOS_WDT_SEM_ERROR;
        FWDT_ERROR("Wait WDT finish notify_result timeout.");
        goto wdt_feed_exit;
    }

    ret = FFreeRTOSWdtControl(os_wdt_ctrl_p, FREERTOS_WDT_CTRL_STOP, NULL);
    if (ret != FWDT_SUCCESS)
    {
        FWDT_ERROR("FFreeRTOSWdtControl CTRL_STOP error.");
        goto wdt_feed_exit;
    }
    

wdt_feed_exit:
    FWDT_INFO("%s return with ret: %d", __func__, ret);
    return ret;
}

static void WdtExampleTask()
{
    FError ret = FWDT_SUCCESS;
    int task_res = WDT_EXAMPLE_SUCCESS;

    ret = WdtExampleInit();
    if (ret != FWDT_SUCCESS)
    {
        task_res = WDT_EXAMPLE_INIT_FAILURE;
        FWDT_ERROR("WdtExampleInit() failed.");
        goto task_exit;
    }

    ret = WdtExampleFeed(); /* 尝试喂几次狗，如果没有出现系统自动复位的现象，则说明喂狗成功，最后等待超时中断产生后退出 */
    if (ret != FWDT_SUCCESS)
    {
        task_res = WDT_EXAMPLE_FEED_FAILURE;
        FWDT_ERROR("WdtExampleFeed() failed.");
        goto task_exit;
    }

task_exit:
    if (os_wdt_ctrl_p != NULL)
    {
        FFreeRTOSWdtDeinit(os_wdt_ctrl_p);
    }

    xQueueSend(xQueue, &task_res, 0);

    vTaskDelete(NULL);
}


int FFreeRTOSWdtCreate(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = WDT_EXAMPLE_UNKNOWN_STATE;

    xQueue = xQueueCreate(1, sizeof(int)); /* 创建消息队列 */
    if (xQueue == NULL)
    {
        FWDT_ERROR("xQueue create failed.");
        goto exit;
    }

    xReturn = xTaskCreate((TaskFunction_t)WdtExampleTask,         /* 任务入口函数 */
                          (const char *)"WdtExampleTask",         /* 任务名字 */
                          4096,                         /* 任务栈大小 */
                          NULL,                                   /* 任务入口函数参数 */
                          (UBaseType_t)WDT_FEED_TASK_PRIORITY,    /* 任务优先级 */
                          NULL);                                  /* 任务句柄 */
    if (xReturn == pdFAIL)
    {
        FWDT_ERROR("xTaskCreate WdtExampleTask failed.");
        goto exit;
    }

    xReturn = xQueueReceive(xQueue, &task_res, WDT_EXAMPLE_TIMEOUT);
    if (xReturn == pdFAIL)
    {
        FWDT_ERROR("xQueue receive timeout.");
        goto exit;
    }

exit:
    if (xQueue != NULL)
    {
        vQueueDelete(xQueue);
    }

    if (task_res != WDT_EXAMPLE_SUCCESS)
    {
        printf("%s@%d: WDT feed example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return task_res;
    }
    else
    {
        printf("%s@%d: WDT feed example [success].\r\n", __func__, __LINE__);
        return task_res;
    }
}