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
 * FilePath: timer_tacho_example.c
 * Date: 2022-08-24 13:57:55
 * LastEditTime: 2022-08-24 13:57:56
 * Description:  This file is for timer tacho example functions.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2022/11/25   init
 */

#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fassert.h"
#include "timers.h"
#include "ftimer_tacho_os.h"
#include "timer_tacho_example.h"
#include "fparameters.h"
#include "fio_mux.h"
#include "fcpu_info.h"
#include "sdkconfig.h"
#include "fdebug.h"


/************************** Constant Definitions *****************************/
#define TACHO_INSTANCE_NUM 1

#define TIMER_TACHO_TEST_TASK_PRIORITY  3

static QueueHandle_t xQueue = NULL;
static boolean is_running = FALSE;
/* The periods assigned to time out */
#define TIMER_OUT                  (pdMS_TO_TICKS(50000UL))
#define TIMER_IRQ_PRIORITY 0xb
#define TACHO_IRQ_PRIORITY 0xc
#define TIMER_INSTANCE_NUM 0U
/* write and read task delay in milliseconds */
#define TASK_DELAY_MS   2000UL
/**************************** Type Definitions *******************************/
enum
{
    TIMER_TACHO_TEST_SUCCESS = 0,
    TIMER_TACHO_TEST_UNKNOWN = 1,
    TIMER_TACHO_INIT_ERROR   = 2,
    TIMER_TACHO_TEST_ERROR = 3,
};
/************************** Variable Definitions *****************************/
static FFreeRTOSTimerTacho *os_timer_ctrl;
static FFreeRTOSTimerTacho *os_tacho_ctrl;

volatile int timerflag = 0;
volatile int tachoflag = 0;
/***************** Macros (Inline Functions) Definitions *********************/
#define FTIMER_TACHO_DEBUG_TAG "TIMER-TACHO"
#define FTIMER_TACHO_ERROR(format, ...) FT_DEBUG_PRINT_E(FTIMER_TACHO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTIMER_TACHO_WARN(format, ...)  FT_DEBUG_PRINT_W(FTIMER_TACHO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTIMER_TACHO_INFO(format, ...)  FT_DEBUG_PRINT_I(FTIMER_TACHO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTIMER_TACHO_DEBUG(format, ...) FT_DEBUG_PRINT_D(FTIMER_TACHO_DEBUG_TAG, format, ##__VA_ARGS__)


/*exit the timer tacho  example task and deinit the gpio */
static void TimerTachoExit(void)
{
    printf("Exiting...\r\n");
    /* deinit iomux */
    FIOMuxDeInit();
    FFreeRTOSTachoDeinit(os_tacho_ctrl);
    os_tacho_ctrl = NULL;
    FFreeRTOSTimerDeinit(os_timer_ctrl);
    os_timer_ctrl = NULL;
    is_running = FALSE;
}

/***** timer intr and handler******/
/**
 * @name: CycCmpIntrHandler
 * @msg: 循环定时回调函数
 * @return {*}
 * @param {void} *param
 */
static void CycCmpIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    timerflag++;
    printf("Cyc intr,id: %d,times_in: %d.\r\n", instance_p->config.id, timerflag);
}

/**
 * @name: OnceCmpIntrHandler
 * @msg: 单次定时回调服务函数
 * @return {*}
 * @param {void} *param
 */
static void OnceCmpIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    printf("Once cmp intr, timer id: %d.\r\n", instance_p->config.id);
    FTimerSetInterruptMask(instance_p, FTIMER_EVENT_ONCE_CMP, FALSE);
}

/**
 * @name: RolloverIntrHandler
 * @msg: 此中断已经在驱动层进行了屏蔽，由于我们设置的cmp值已经是翻转值，所以等同于中断计数中断，此处可作为用法的拓展
 * @return {*}
 * @param {void} *param
 */
static void RolloverIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    /* Anything else that you can do.*/
    printf("Roll over cmp intr, timer id: %d", instance_p->config.id);
}

/**
 * @name: TimerDisableIntr
 * @msg: 失能中断
 * @return {void}
 * @param {FTimerTachoCtrl} *instance_p 驱动控制数据结构
 */
void TimerDisableIntr(FTimerTachoCtrl *instance_p)
{
    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);

    InterruptMask(irq_num);
}

/**
 * @name: TimerEnableIntr
 * @msg: 设置并且使能中断
 * @return {void}
 * @param  {FTimerTachoCtrl} *instance_p 驱动控制数据结构
 */
static void TimerEnableIntr(FTimerTachoCtrl *instance_p)
{
    FASSERT(instance_p);

    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);

    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(irq_num, cpu_id);
    /* disable timer irq */
    InterruptMask(irq_num);

    /* umask timer irq */
    InterruptSetPriority(irq_num, TIMER_IRQ_PRIORITY);
    InterruptInstall(irq_num, FTimerTachoIntrHandler, instance_p, instance_p->config.name);

    FTimerTachoSetIntr(instance_p);
    /* enable irq */
    InterruptUmask(irq_num);

    return ;
}

/***** tacho intr and handler******/

static void TachoDisableIntr(FTimerTachoCtrl *instance_p)
{
    FASSERT(instance_p);
    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);
    InterruptMask(irq_num);
}

static void TachOverIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);
    InterruptMask(irq_num);
    u32 rpm;
    FTachoGetFanRPM(instance_p, &rpm);
    printf("TachOver intr,tacho id: %d,rpm:%d.\r\n", instance_p->config.id, rpm);
    InterruptUmask(irq_num);
    tachoflag++;
    if (tachoflag > 20)
    {
        tachoflag = 0;
        TachoDisableIntr(instance_p);
        printf("Please deinit tacho,then init.");
    }
}

static void CapIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    printf("TachCapt intr,tacho id: %d", instance_p->config.id);
}

static void TachUnderIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);
    InterruptMask(irq_num);
    u32 rpm;
    FTachoGetFanRPM(instance_p, &rpm);
    printf("TachUnder intr,tacho id: %d,rpm:%d.\r\n", instance_p->config.id, rpm);
    InterruptUmask(irq_num);
    tachoflag++;
    if (tachoflag > 20)
    {
        tachoflag = 0;
        TachoDisableIntr(instance_p);
        printf("Please deinit tacho,then init again.");
    }
}

void TachoEnableIntr(FTimerTachoCtrl *instance_p)
{
    FASSERT(instance_p);
    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);

    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(irq_num, cpu_id);

    /* disable timer irq */
    InterruptMask(irq_num);

    /* umask timer irq */
    InterruptSetPriority(irq_num, TACHO_IRQ_PRIORITY);
    InterruptInstall(irq_num, FTimerTachoIntrHandler, instance_p, instance_p->config.name);

    FTimerTachoSetIntr(instance_p);
    /* enable irq */
    InterruptUmask(irq_num);

    return;
}

/*tacho test function*/
static FError TachoTest(void)
{
    FError ret = FT_SUCCESS;
    u32 rpm;
    TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    vTaskDelay(xDelay);
    printf("\r\n*****TachoTask is running...\r\n");
    TachoEnableIntr(&os_tacho_ctrl->ctrl);
    ret = FFreeRTOSTimerStart(os_tacho_ctrl);

    vTaskDelay(100);/*等待采样周期完成*/
    if (ret != FT_SUCCESS)
    {
        ret = TIMER_TACHO_TEST_ERROR;
        goto exit;
    }
    for (size_t i = 0; i < 5; i++)
    {
        ret = FFreeRTOSTachoGetRPM(os_tacho_ctrl, &rpm);
        if (ret != FT_SUCCESS)
        {
            printf("TachoTask Stop failed.\r\n");
            goto exit;
        }
        printf("***GET_RPM:%d.\r\n", rpm);
        vTaskDelay(xDelay);/*Collect every 2 seconds*/
    }

exit:
    /* disable tacho irq */
    TachoDisableIntr(&os_tacho_ctrl->ctrl);
    /* disable tacho */
    FFreeRTOSTachoDeinit(os_tacho_ctrl);
    return ret;
}

/*timer test function*/
static FError TimerTest(void)
{
    FError ret = FT_SUCCESS;
    TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    vTaskDelay(xDelay);
    printf("\r\n*****TimerTask is running...\r\n");

    TimerEnableIntr(&os_timer_ctrl->ctrl);
    ret = FFreeRTOSTimerStart(os_timer_ctrl);
    if (ret != FT_SUCCESS)
    {
        ret = TIMER_TACHO_TEST_ERROR;
        goto exit;
    }
    xDelay = pdMS_TO_TICKS(10000);/*delay 10s*/
    vTaskDelay(xDelay);
    ret = FFreeRTOSTimerStop(os_timer_ctrl);
    if (ret != FT_SUCCESS)
    {
        ret = TIMER_TACHO_TEST_ERROR;
        goto exit;
    }

exit:
    /* disable timer irq */
    TimerDisableIntr(&os_timer_ctrl->ctrl);
    /* disable timer  */
    FFreeRTOSTimerDeinit(os_timer_ctrl);
    return ret;
}

/*timer init function*/
static FError TimerInit(void)
{
    FError ret = FT_SUCCESS;
    os_timer_ctrl = FFreeRTOSTimerInit(TIMER_INSTANCE_NUM, FTIMER_CYC_CMP, 2000000);/* 2000000 us = 2 s */

    if (os_timer_ctrl == NULL)
    {
        ret = TIMER_TACHO_INIT_ERROR;
        goto exit;
    }
    FTimerRegisterEvtCallback(&os_timer_ctrl->ctrl, FTIMER_EVENT_CYC_CMP, CycCmpIntrHandler);
    FTimerRegisterEvtCallback(&os_timer_ctrl->ctrl, FTIMER_EVENT_ONCE_CMP, OnceCmpIntrHandler);
    FTimerRegisterEvtCallback(&os_timer_ctrl->ctrl, FTIMER_EVENT_ROLL_OVER, RolloverIntrHandler);
    /*init mode: FTIMER_WORK_MODE_CAPTURE or FTIMER_WORK_MODE_TACHO */
    os_tacho_ctrl = FFreeRTOSTachoInit(TACHO_INSTANCE_NUM, FTIMER_WORK_MODE_TACHO);
    if (os_tacho_ctrl == NULL)
    {
        ret = TIMER_TACHO_INIT_ERROR;
        goto exit;
    }
    /*init iomux*/
    FIOMuxInit();
    /* set iopad mux */
    FIOPadSetTachoMux(TACHO_INSTANCE_NUM);

    FTimerRegisterEvtCallback(&os_tacho_ctrl->ctrl, FTACHO_EVENT_OVER, TachOverIntrHandler);
    FTimerRegisterEvtCallback(&os_tacho_ctrl->ctrl, FTACHO_EVENT_UNDER, TachUnderIntrHandler);
    FTimerRegisterEvtCallback(&os_tacho_ctrl->ctrl, FTACHO_EVENT_CAPTURE, CapIntrHandler);

exit:
    return ret;
}

/*timer tacho test task*/
static void TimerTachoTask(void)
{
    int task_res = TIMER_TACHO_TEST_SUCCESS;
    FError ret = FT_SUCCESS;
    ret = TimerInit();

    if (ret != FT_SUCCESS)
    {
        FTIMER_TACHO_ERROR("TimerTachoTask failed.");
        task_res = TIMER_TACHO_INIT_ERROR;
        goto task_exit;
    }
    /* create timer test */
    ret = TimerTest();
    if (ret != FT_SUCCESS)
    {
        FTIMER_TACHO_ERROR("TimerTest failed.");
        task_res = TIMER_TACHO_TEST_ERROR;
        goto task_exit;
    }
    /* create tacho test */
    ret = TachoTest();
    if (ret != FT_SUCCESS)
    {
        FTIMER_TACHO_ERROR("TachoTest failed.");
        task_res = TIMER_TACHO_TEST_ERROR;
        goto task_exit;
    }

task_exit:
    xQueueSend(xQueue, &task_res, 0);
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSTimerTachoCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

    int task_res = TIMER_TACHO_TEST_UNKNOWN;
    if (is_running)
    {
        FTIMER_TACHO_ERROR("The task is running.");
        return pdPASS;
    }
    is_running = TRUE;
    /* init timers controller */
    xQueue = xQueueCreate(1, sizeof(int)); /* create queue for task communication */
    if (xQueue == NULL)
    {
        FTIMER_TACHO_ERROR("xQueue create failed.");
        goto exit;
    }

    xReturn = xTaskCreate((TaskFunction_t)TimerTachoTask,  /* 任务入口函数 */
                          (const char *)"TimerTachoTask",/* 任务名字 */
                          1024,  /* 任务栈大小 */
                          (void *)NULL,/* 任务入口函数参数 */
                          (UBaseType_t)TIMER_TACHO_TEST_TASK_PRIORITY, /* 任务的优先级 */
                          NULL);                                  /* 任务句柄 */
    if (xReturn == pdFAIL)
    {
        FTIMER_TACHO_ERROR("xTaskCreate TimerTachoTask failed.");
        goto exit;
    }
    xReturn = xQueueReceive(xQueue, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        FTIMER_TACHO_ERROR("xQueue receive timeout.");
        goto exit;
    }

exit:
    if (xQueue != NULL)
    {
        vQueueDelete(xQueue);
    }

    if (task_res != TIMER_TACHO_TEST_SUCCESS)
    {
        printf("%s@%d: Timer tacho example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: Timer tacho example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}

