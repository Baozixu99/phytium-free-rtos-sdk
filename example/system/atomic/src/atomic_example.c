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
 * FilePath: atomic_example.c
 * Date: 2023-02-23 14:53:42
 * LastEditTime: 2023-03-01 17:57:36
 * Description:  This file is for atomic test function.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangxiaodong 2023/06/23	  first release
 * 1.1  zhangyan     2024/4/29    add no letter shell mode, adapt to auto-test system
 */

#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "fatomic.h"
#include "fkernel.h"

#define TIMER_OUT (pdMS_TO_TICKS(1000UL))
#define TASK_STACK_SIZE         1024
#define ATOMIC_TEST_TASK_PRIORITY 3

enum
{
    ATOMIC_TEST_SUCCESS = 0,
    ATOMIC_TEST_UNKNOWN = 1,
    ATOMIC_TEST_FAILURE = 2,
};
static QueueHandle_t xQueue = NULL;

void FAtomicExampleTask(void *pvParameters)
{
    int task_res = ATOMIC_TEST_SUCCESS;
    int ret;
    int i = 0;
    u32 count = 0;
    u32 times = 0;

    while (i++ < 10)
    {
        ret = FATOMIC_ADD(count, 1);
    }
    if (count != 10)
    {
        printf("FATOMIC_ADD error\r\n");
        task_res = ATOMIC_TEST_FAILURE;
        goto task_exit;
    }
    
    i = 0;
    while (i++ < 10) 
    {
        ret = FATOMIC_INC(count);
    }
    if (count != 20)
    {
        printf("FATOMIC_INC error\r\n");
        task_res = ATOMIC_TEST_FAILURE;
        goto task_exit;
    }

    i = 0;
    while (i++ < 10) 
    {
        ret = FATOMIC_SUB(count, 1);
    }
    if (count != 10)
    {
        printf("FATOMIC_SUB error\r\n");
        task_res = ATOMIC_TEST_FAILURE;
        goto task_exit;
    }

    i = 0;
    while (i++ < 10) 
    {
        ret = FATOMIC_DEC(count);
    }
    if (count != 0)
    {
        printf("FATOMIC_DEC error\r\n");
        task_res = ATOMIC_TEST_FAILURE;
        goto task_exit;
    }

    i = 0;
    count = 0;
    while (i++ < 16)
    {
        ret = FATOMIC_OR(count, BIT(i-1));
    }
    if (count != 0xFFFF)
    {
        printf("FATOMIC_OR error\r\n");
        task_res = ATOMIC_TEST_FAILURE;
        goto task_exit;
    }

    i = 0;
    count = 0xFFFF;
    while (i++ < 16)
    {
        ret = FATOMIC_AND(count, ~BIT(i-1));
    }
    if (count != 0)
    {
        printf("FATOMIC_AND error\r\n");
        task_res = ATOMIC_TEST_FAILURE;
        goto task_exit;
    }

    FATOMIC_CAS_BOOL(count, 0, 1);
    if (count != 1)
    {
        printf("FATOMIC_CAS_BOOL error\r\n");
        task_res = ATOMIC_TEST_FAILURE;
        goto task_exit;
    }

    FATOMIC_CAS_VAL(count, 0xFF, 0);
    if (count != 1)
    {
        printf("FATOMIC_CAS_VAL error\r\n");
        task_res = ATOMIC_TEST_FAILURE;
        goto task_exit;
    }

    FATOMIC_LOCK(count, 1);
    if (count != 1)
    {
        printf("FATOMIC_LOCK error\r\n");
        task_res = ATOMIC_TEST_FAILURE;
        goto task_exit;
    }

    FATOMIC_UNLOCK(count);
    if (count != 0)
    {
        printf("FATOMIC_UNLOCK error\r\n");
        task_res = ATOMIC_TEST_FAILURE;
        goto task_exit;
    }

task_exit:
    xQueueSend(xQueue, &task_res, 0);

    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSAtomicTaskCreate(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = ATOMIC_TEST_UNKNOWN;

    xQueue = xQueueCreate(1, sizeof(int));
    if (xQueue == NULL)
    {
        printf("xQueue create failed.\r\n");
        goto exit;
    }

    xReturn = xTaskCreate((TaskFunction_t)FAtomicExampleTask, /* 任务入口函数 */
                          (const char *)"FAtomicExampleTask", /* 任务名字 */
                          TASK_STACK_SIZE,                            /* 任务栈大小 */
                          NULL,                                      /* 任务入口函数参数 */
                          (UBaseType_t)ATOMIC_TEST_TASK_PRIORITY,     /* 任务的优先级 */
                          NULL);
    if (xReturn == pdFAIL)
    {
        printf("xTaskCreate FAtomicExampleTask failed.");
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
    if (task_res != ATOMIC_TEST_SUCCESS)
    {
        printf("%s@%d: Atomic test example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: Atomic test indirect example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}