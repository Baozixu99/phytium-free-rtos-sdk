/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-07-05 09:06:14
 * @LastEditTime: 2021-07-05 13:24:26
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ft_types.h"

const UBaseType_t task1Priority = configKERNEL_INTERRUPT_PRIORITY - 1;
const UBaseType_t task2Priority = configKERNEL_INTERRUPT_PRIORITY/2;
const UBaseType_t task3Priority = configKERNEL_INTERRUPT_PRIORITY/3;
const TickType_t task1Delay = 100;
const TickType_t task2Delay = 100;
const TickType_t task3Delay = 100;
const u16 taskStackDepth = 256;

void Task1(void *param)
{
    while (1)
    {
        printf("task1!!!\r\n");
        vTaskDelay(task1Delay);
    }
}

void Task2(void *param)
{
    while (1)
    {
        printf("task2!!!\r\n");
        vTaskDelay(task2Delay);
    }
}

void Task3(void *param)
{
    while (1)
    {
        printf("task3!!!\r\n");
        vTaskDelay(task3Delay);
    }
}

void TestTaskPriority()
{
    if (xTaskCreate(Task1, "task1", taskStackDepth, NULL, task1Priority, NULL) != pdPASS)
    {
        printf("create task1 failed \r\n");
    }    

    if (xTaskCreate(Task2, "task2", taskStackDepth, NULL, task2Priority, NULL) != pdPASS)
    {
        printf("create task2 failed \r\n");
    } 

    if (xTaskCreate(Task3, "task3", taskStackDepth, NULL, task3Priority, NULL) != pdPASS)
    {
        printf("create task3 failed \r\n");
    } 
}