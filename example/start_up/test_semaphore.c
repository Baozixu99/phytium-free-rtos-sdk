/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-07-05 14:00:50
 * @LastEditTime: 2021-07-05 14:15:44
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "ft_types.h"
#include "ft_assert.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

static TaskHandle_t AppTaskCreate_Handle = NULL;/* 创建任务句柄 */
static TaskHandle_t Receive_Task_Handle = NULL; /* 接收任务句柄 */
static TaskHandle_t Send_Task_Handle = NULL;    /* 发送任务句柄 */
SemaphoreHandle_t BinarySem_Handle =NULL;

static void Receive_Task(void* parameter) 
{ 
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为 pdTRUE */ 
    uint32_t r_queue; /* 定义一个接收消息的变量 */ 
    while (1)
    { 
        xReturn = xSemaphoreTake(BinarySem_Handle,/* 二值信号量句柄 */ 
                                 portMAX_DELAY); /* 等待时间 */
        if (pdTRUE == xReturn) 
            printf("Semaphore Got!!\r\n"); 
    } 
}

extern u8 BlockReceive();
extern void Uart1Init(void);
static void Send_Task(void* parameter)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    u8 Byte;

    while (1)
    {
        printf("Wait for input...\r\n");
        if (Byte = BlockReceive())
        {
            xReturn = xSemaphoreGive(BinarySem_Handle);//给出二值信号量

            if (pdPASS == xReturn)
            {
                printf("Semaphore Released!!\r\n");
            }

            vTaskDelay(20);/* 延时 20 个 tick */
        }
    }
}

static void AppTaskCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

    taskENTER_CRITICAL(); //进入临界区

    BinarySem_Handle = xSemaphoreCreateBinary();

    FT_ASSERTVOID(NULL != BinarySem_Handle);

    /* 创建任务 */
    xReturn = xTaskCreate((TaskFunction_t )Receive_Task, /* 任务入口函数 */
                            (const char* )"Receive_Task",/* 任务名字 */
                            (uint16_t )512, /* 任务栈大小 */
                            (void* )NULL, /* 任务入口函数参数 */
                            (UBaseType_t )2, /* 任务的优先级 */
                            (TaskHandle_t* )&Receive_Task_Handle);/* 任务控制块指针 */

    FT_ASSERTVOID(pdPASS == xReturn);

    xReturn = xTaskCreate((TaskFunction_t )Send_Task, /* 任务入口函数 */
                        (const char* )"Send_Task",/* 任务名字 */
                        (uint16_t )512, /* 任务栈大小 */
                        (void* )NULL, /* 任务入口函数参数 */
                        (UBaseType_t )2, /* 任务的优先级 */
                        (TaskHandle_t* )&Send_Task_Handle);/* 任务控制块指针 */

    FT_ASSERTVOID(pdPASS == xReturn);

    vTaskDelete(AppTaskCreate_Handle); //删除 AppTaskCreate 任务

    taskEXIT_CRITICAL(); //退出临界区
}


void TestSemaphoreEntry()
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

    Uart1Init();

    printf("get cpu stats \r\n");
    xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate, /* 任务入口函数 */
                            (const char* )"AppTaskCreate",/* 任务名字 */
                            (uint16_t )512, /* 任务栈大小 */
                            (void* )NULL,/* 任务入口函数参数 */
                            (UBaseType_t )1, /* 任务的优先级 */
                            (TaskHandle_t* )&AppTaskCreate_Handle);/* 任务控制块指针
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