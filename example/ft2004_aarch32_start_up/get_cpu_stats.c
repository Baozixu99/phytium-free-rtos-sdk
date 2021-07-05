/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-07-05 10:28:22
 * @LastEditTime: 2021-07-05 11:36:56
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

static u32 flag1 = 0;
#define TRACE_FLAG(flag, id) printf("TURN %s FLAG %d\r\n", (flag) ? "ON": "OFF", (id))
static TaskHandle_t flag1TaskHandle = NULL;
static u32 Buff1[256];
static void Flag1Task(void* parameter)
{
    while(1)
    {
        flag1 = 1;
        TRACE_FLAG(flag1, 1);
        memset(Buff1, 0, 256);
        for (u32 i = 0; i < 256; i++)
        {
            Buff1[i] = i + 1;
        }
        vTaskDelay(500);
        flag1 = 0;
        TRACE_FLAG(flag1, 1);
        memset(Buff1, 0, 256);
        for (u32 i = 0; i < 256; i++)
        {
            Buff1[i] = i + 2;
        }        
        vTaskDelay(500);            
    }
}


static u32 flag2 = 0;
static TaskHandle_t flag2TaskHandle = NULL;
static u32 Buff2[512];
static void Flag2Task(void* parameter)
{
    while (1)
    {
        flag2 = 1;
        TRACE_FLAG(flag2, 2);
        memset(Buff2, 0, 512);
        for (u32 i = 0; i < 512; i++)
        {
            Buff2[i] = i + 1;
        }        
        vTaskDelay(500);
        flag2 = 0;
        TRACE_FLAG(flag2, 2);
        memset(Buff2, 0, 512);
        for (u32 i = 0; i < 512; i++)
        {
            Buff2[i] = i + 2;
        }         
        vTaskDelay(500);   
    } 
}

static TaskHandle_t cpuStatsTaskHandle = NULL;
static void CpuStatsTask(void* parameter)
{
    uint8_t CPU_RunInfo[400]; //保存任务运行时间信息

    while (1) 
    {
        memset(CPU_RunInfo,0,400); //信息缓冲区清零
        vTaskList((char *)&CPU_RunInfo); //获取任务运行时间信息

        printf("---------------------------------------------\r\n"); 
        printf("任务名\t任务状态\t优先级\t剩余栈\t任务序号\r\n"); 
        printf("%s", CPU_RunInfo); 
        printf("---------------------------------------------\r\n");

        memset(CPU_RunInfo, 0, 400); //信息缓冲区清零

        vTaskGetRunTimeStats((char *)&CPU_RunInfo);

        printf("任务名\t运行计数\t使用率\r\n"); 
        printf("%s", CPU_RunInfo); 
        printf("---------------------------------------------\r\n\n"); 
        vTaskDelay(2000); /* 延时 */        
    }
}

static TaskHandle_t appTaskCreateHandle = NULL;
static void AppTaskCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

    taskENTER_CRITICAL(); //进入临界区

    /* 创建 Flag_Task 任务 */
    xReturn = xTaskCreate((TaskFunction_t )Flag1Task, /* 任务入口函数 */
                            (const char* )"FLAG1_Task",/* 任务名字 */
                            (uint16_t )512, /* 任务栈大小 */
                            (void* )NULL, /* 任务入口函数参数 */
                            (UBaseType_t )2, /* 任务的优先级 */
                            (TaskHandle_t* )&flag1TaskHandle);/* 任务控制块指针 */    

    if (pdPASS == xReturn)
    {
        printf("create flag1 task success!\r\n");  
    }  

    /* 创建 Flag_Task 任务 */
    xReturn = xTaskCreate((TaskFunction_t )Flag2Task, /* 任务入口函数 */
                            (const char* )"FLAG2_Task",/* 任务名字 */
                            (uint16_t )512, /* 任务栈大小 */
                            (void* )NULL, /* 任务入口函数参数 */
                            (UBaseType_t )2, /* 任务的优先级 */
                            (TaskHandle_t* )&flag2TaskHandle);/* 任务控制块指针 */    

    if (pdPASS == xReturn)
    {
        printf("create flag2 task success!\r\n");  
    }      

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

void TestCpuStatsEntry()
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    
    printf("get cpu stats \r\n");
    xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate, /* 任务入口函数 */
                            (const char* )"AppTaskCreate",/* 任务名字 */
                            (uint16_t )512, /* 任务栈大小 */
                            (void* )NULL,/* 任务入口函数参数 */
                            (UBaseType_t )1, /* 任务的优先级 */
                            (TaskHandle_t* )&appTaskCreateHandle);/* 任务控制块指针
    /* 启动任务调度 */
    if (pdPASS == xReturn)
    {
        vTaskStartScheduler(); /* 启动任务，开启调度 */        
    }
    else
    {
        return;        
    }

    while (1); /* 正常不会执行到这里 */
}