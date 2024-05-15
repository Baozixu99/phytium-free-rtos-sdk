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
 * FilePath: nested_interrupt.c
 * Date: 2023-02-23 14:53:42
 * LastEditTime: 2023-03-01 17:57:36
 * Description:  This file is for nested interrupt test function.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangxiaodong 2023/2/23	  first release
 * 1.1  zhangyan     2024/4/29    add no letter shell mode, adapt to auto-test system
 */

#include <stdio.h>
#include <math.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "croutine.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fexception.h"

enum
{
    NESTED_INTR_TEST_SUCCESS = 0,
    NESTED_INTR_TEST_UNKNOWN = 1,
    NESTED_INTR_TEST_FAILURE = 2,
};
static QueueHandle_t xQueue = NULL;

#define NESTED_INTR_TEST_TASK_PRIORITY 3

#define TIMER_OUT (pdMS_TO_TICKS(1000UL))
#define DELAT_PER_TEST (pdMS_TO_TICKS(100UL))
#define TEST_TIMES 5

#define TASK_STACK_SIZE         1024

/* The interrupt number to use for the software interrupt generation.  This
could be any unused number.  In this case the first chip level (non system)
interrupt is used */
#define INTERRUPT_LOW_ID         0
#define INTERRUPT_HIGH_ID        1

/* The priority of the software interrupt.  The interrupt service routine uses
an (interrupt safe) FreeRTOS API function, so the priority of the interrupt must
be equal to or lower than the priority set by
configMAX_SYSCALL_INTERRUPT_PRIORITY - remembering that on the Cortex M3 high
numeric values represent low priority values, which can be confusing as it is
counter intuitive. */
#define INTERRUPT_LOW_PRIORITY  IRQ_PRIORITY_VALUE_14
#define INTERRUPT_HIGH_PRIORITY  (INTERRUPT_LOW_PRIORITY-1)

/* Macro to force an interrupt. */
static void vTriggerNestedInterrupt(void);

static u32 cpu_id = 0;
static volatile u8 high_priority_intr_flag = 0;	/* Flag to update high priority interrupt counter */

/*The initial value of the calculated values for high priority and low priority tasks*/
float val_low = 1.0;
float val_high = 1.0;

/*The final correct calculation result value*/
#define VAL_HIGH_CALC_RES (float)1.104081
#define VAL_LOW_CALC_RES (float)1.051010


static void vNestedPeriod(void)
{
    u8 count = 0;
    for (count = 0; count < TEST_TIMES; count++)
    {
        vTriggerNestedInterrupt();
        vTaskDelay(DELAT_PER_TEST);
    }
}

static void FLowPriorityHandlerFunc(void)
{
    val_low = val_low * 1.01;

    /* Activate high-priority intr */
    InterruptCoreInterSend(INTERRUPT_HIGH_ID, (1 << cpu_id));

	/* Wait till interrupts from counter configured with high priority interrupt */
	while(high_priority_intr_flag == 0);

    printf("val_low = %f \r\n", val_low);
}

static void FLowPriorityHandler(s32 vector, void *param)
{
    static fsize_t value[3] = {0};

	/* Enable the nested interrupts to allow preemption */     
    FInterruptNestedEnable(value);

    /* A function operation must be used between interrupt nesting enable and disable */
    FLowPriorityHandlerFunc();

    /* Disable the nested interrupt before exiting IRQ mode */
    FInterruptNestedDisable(value);
}

static void FHighPriorityHandlerFunc(void)
{
    high_priority_intr_flag++;
    val_high = val_high * 1.02;

    printf("val_high = %f \r\n", val_high);
}

static void FHighPriorityHandler(s32 vector, void *param)
{
    static fsize_t value[3] = {0};

    /* Enable the nested interrupts to allow preemption */     
    FInterruptNestedEnable(value);
    /* A function operation must be used between interrupt nesting enable and disable */
    FHighPriorityHandlerFunc();
    /* Disable the nested interrupt before exiting IRQ mode */
    FInterruptNestedDisable(value);
}

static void prvSetupSoftwareNestedInterrupt()
{
    GetCpuId(&cpu_id);
    /* The interrupt service routine uses an (interrupt safe) FreeRTOS API
    function so the interrupt priority must be at or below the priority defined
    by configSYSCALL_INTERRUPT_PRIORITY. */
    InterruptSetPriority(INTERRUPT_LOW_ID, INTERRUPT_LOW_PRIORITY);
    InterruptInstall(INTERRUPT_LOW_ID, FLowPriorityHandler, NULL, NULL);
    InterruptUmask(INTERRUPT_LOW_ID);/* Enable the interrupt. */


    InterruptSetPriority(INTERRUPT_HIGH_ID, INTERRUPT_HIGH_PRIORITY);
    InterruptInstall(INTERRUPT_HIGH_ID, FHighPriorityHandler, NULL, NULL);
    InterruptUmask(INTERRUPT_HIGH_ID);    /* Enable the interrupt. */
}

/* Macro to force an interrupt. */
static void vTriggerNestedInterrupt(void)
{
    high_priority_intr_flag = 0;
    /* Activate low-priority intr */
    InterruptCoreInterSend(INTERRUPT_LOW_ID, (1 << cpu_id));
}

void NestedIntrTask(void)
{
    int task_res = NESTED_INTR_TEST_SUCCESS;
    prvSetupSoftwareNestedInterrupt();
    vNestedPeriod();
    /*取1E-6精度下比较计算结果，此精度可修改*/
    if (fabs(val_low - VAL_LOW_CALC_RES) >= 1E-6 || fabs(val_high - VAL_HIGH_CALC_RES) >= 1E-6)
    {
        task_res = NESTED_INTR_TEST_FAILURE;
    }

    xQueueSend(xQueue, &task_res, 0);

    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSNestedIntrTaskCreate(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = NESTED_INTR_TEST_UNKNOWN;

    xQueue = xQueueCreate(1, sizeof(int));
    if (xQueue == NULL)
    {
        printf("xQueue create failed.\r\n");
        goto exit;
    }

    xReturn = xTaskCreate((TaskFunction_t)NestedIntrTask, /* 任务入口函数 */
                          (const char *)"NestedIntrTask", /* 任务名字 */
                          (uint16_t)TASK_STACK_SIZE,                            /* 任务栈大小 */
                          NULL,                                      /* 任务入口函数参数 */
                          (UBaseType_t)NESTED_INTR_TEST_TASK_PRIORITY,     /* 任务的优先级 */
                          NULL);
    if (xReturn == pdFAIL)
    {
        printf("xTaskCreate NestedIntrTask failed.");
        goto exit;
    }

    xReturn = xQueueReceive(xQueue, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        printf("xQueue receive timeout.\r\n");
        goto exit;
    }

exit:
    vQueueDelete(xQueue);
    if (task_res != NESTED_INTR_TEST_SUCCESS)
    {
        printf("%s@%d: Nested intr test example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: Nested intr test indirect example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}