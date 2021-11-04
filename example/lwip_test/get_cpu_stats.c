/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-07-05 10:28:22
 * @LastEditTime: 2021-07-22 09:05:53
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
#include "ft_types.h"

static TaskHandle_t cpuStatsTaskHandle = NULL;
static void CpuStatsTask(void* parameter)
{
    u8 CPU_RunInfo[400]; //保存任务运行时间信息

    while (1) 
    {
        memset(CPU_RunInfo,0,400); //信息缓冲区清零
        vTaskList((char *)&CPU_RunInfo); //获取任务运行时间信息

        printf("---------------------------------------------\r\n"); 
        printf("task_name  task_state  priority  stack  task_num\r\n"); 
        printf("%s", CPU_RunInfo); 
        printf("---------------------------------------------\r\n");

        memset(CPU_RunInfo, 0, 400); //信息缓冲区清零

        vTaskGetRunTimeStats((char *)&CPU_RunInfo);

        printf("task_name\trun_time_count\tusage_rate\r\n"); 
        printf("%s", CPU_RunInfo); 
        printf("---------------------------------------------\r\n\n"); 
        vTaskDelay(2000); /* 延时 */        
    }
}

static TaskHandle_t appTaskCreateHandle = NULL;
static void AppTaskCreate(void)
{
    BaseType_t ret = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
 
    taskENTER_CRITICAL(); //进入临界区
 
    /* 创建 CPU stats 任务 */
    ret = xTaskCreate((TaskFunction_t )CpuStatsTask, /* 任务入口函数 */
                        (const char* )"CPU_STATS_Task",/* 任务名字 */
                        (uint16_t )1024, /* 任务栈大小 */
                        (void* )NULL, /* 任务入口函数参数 */
                        (UBaseType_t )4, /* 任务的优先级 */
                        (TaskHandle_t* )&cpuStatsTaskHandle);/* 任务控制块指针 */
    
    if (pdPASS == ret)
    {
        printf("create cpu stats task success!\r\n");  
    }   

    vTaskDelete(appTaskCreateHandle); //删除 AppTaskCreate 任务

    taskEXIT_CRITICAL(); //退出临界区
}

BaseType_t TestCpuStatsEntry()
{
    BaseType_t ret = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    
    ret = xTaskCreate((TaskFunction_t )AppTaskCreate, /* 任务入口函数 */
                            (const char* )"AppTaskCreate",/* 任务名字 */
                            (uint16_t )512, /* 任务栈大小 */
                            (void* )NULL,/* 任务入口函数参数 */
                            (UBaseType_t )1, /* 任务的优先级 */
                            (TaskHandle_t* )&appTaskCreateHandle); /* 任务控制 */
                            
    return ret;
}