/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-07-09 08:08:39
 * @LastEditTime: 2021-07-09 13:55:27
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "ft_types.h"
#include "ft_assert.h"
#include "gicv3.h"
#include "generic_timer.h"
#include "interrupt.h"
#include "parameters.h"

static TaskHandle_t cpuStatsTaskHandle = NULL;

static void CpuStatsTask(void* parameter)
{
    u32 count = 0;

    while (1) 
    {
        printf("hello ft Date: %s, Time: %s, count=%d\n", __DATE__, __TIME__, count++);
        vTaskDelay(2000); /* 延时 */        
    }
}

static TaskHandle_t appTaskCreateHandle = NULL;

static void AppTaskCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

    printf("create AppTaskCreate!\r\n");  
    taskENTER_CRITICAL(); //进入临界区
    /* 创建 CPU stats 任务 */
    xReturn = xTaskCreate((TaskFunction_t )CpuStatsTask, /* 任务入口函数 */
                        (const char* )"CPU_STATS_Task",/* 任务名字 */
                        (uint16_t )512, /* 任务栈大小 */
                        (void* )NULL, /* 任务入口函数参数 */
                        (UBaseType_t )4, /* 任务的优先级 */
                        (TaskHandle_t* )&cpuStatsTaskHandle);/* 任务控制块指针 */    

    if (pdPASS == xReturn)
    {
        printf("create cpu stats task success!\r\n");  
    }   

    vTaskDelete(appTaskCreateHandle); //删除 AppTaskCreate 任务

    taskEXIT_CRITICAL(); //退出临界区
}

BaseType_t TestFreeRTOSEntry()
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    
    printf("get cpu stats TestCpuStatsEntry\r\n");
    xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate, /* 任务入口函数 */
                            (const char* )"AppTaskCreate",/* 任务名字 */
                            (uint16_t )512, /* 任务栈大小 */
                            (void* )NULL,/* 任务入口函数参数 */
                            (UBaseType_t )1, /* 任务的优先级 */
                            (TaskHandle_t* )&appTaskCreateHandle); /* 任务控制 */
                            
    return xReturn;
}

int main()
{

    printf("main hello ft Date: %s, Time: %s\n", __DATE__, __TIME__);
   
    BaseType_t xReturn = pdPASS;

    /* 创建 CPU stats 任务 */
    xReturn = TestFreeRTOSEntry();

    if (pdPASS == xReturn)
    {
        printf("create cpu stats task success!\r\n");  
    }   

    vTaskStartScheduler(); /* 启动任务，开启调度 */   

    while (1); /* 正常不会执行到这里 */

    return 0;
}