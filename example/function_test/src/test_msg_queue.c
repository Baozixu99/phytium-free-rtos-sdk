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
 * FilePath: test_msg_queue.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:01:43
 * Description:  This file is for 
 * 
 * Modify History: 
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


static TaskHandle_t AppTaskCreate_Handle = NULL;/* 创建任务句柄 */
static TaskHandle_t Receive_Task_Handle = NULL; /* 接收任务句柄 */
static TaskHandle_t Send_Task_Handle = NULL;    /* 发送任务句柄 */
QueueHandle_t Test_Queue = NULL;
#define QUEUE_LEN 4 /* 队列的长度，最大可包含多少个消息 */
#define QUEUE_SIZE 4 /* 队列中每个消息大小（字节） */

static void AppTaskCreate(void); /* 用于创建任务 */
static void Receive_Task(void* pvParameters);/* Receive_Task 任务实现 */
static void Send_Task(void* pvParameters);/* Send_Task 任务实现 */

static void AppTaskCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

    taskENTER_CRITICAL(); //进入临界区

    Test_Queue = xQueueCreate((UBaseType_t ) QUEUE_LEN, /* 消息队列的长度 */
                              (UBaseType_t ) QUEUE_SIZE);/* 消息的大小 */

    FASSERT(NULL != Test_Queue);

    /* 创建任务 */
    xReturn = xTaskCreate((TaskFunction_t )Receive_Task, /* 任务入口函数 */
                            (const char* )"Receive_Task",/* 任务名字 */
                            (uint16_t )1024, /* 任务栈大小 */
                            (void* )NULL, /* 任务入口函数参数 */
                            (UBaseType_t )2, /* 任务的优先级 */
                            (TaskHandle_t* )&Receive_Task_Handle);/* 任务控制块指针 */

    FASSERT(pdPASS == xReturn);

    xReturn = xTaskCreate((TaskFunction_t )Send_Task, /* 任务入口函数 */
                        (const char* )"Send_Task",/* 任务名字 */
                        (uint16_t )1024, /* 任务栈大小 */
                        (void* )NULL, /* 任务入口函数参数 */
                        (UBaseType_t )2, /* 任务的优先级 */
                        (TaskHandle_t* )&Send_Task_Handle);/* 任务控制块指针 */

    FASSERT(pdPASS == xReturn);

    vTaskDelete(AppTaskCreate_Handle); //删除 AppTaskCreate 任务

    taskEXIT_CRITICAL(); //退出临界区
}

static void Receive_Task(void* parameter) 
{ 
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为 pdTRUE */ 
    uint32_t r_queue; /* 定义一个接收消息的变量 */ 
    while (1)
    { 
        xReturn = xQueueReceive(Test_Queue, /* 消息队列的句柄 */ 
                                &r_queue, /* 发送的消息内容 */ 
                                portMAX_DELAY); /* 等待时间一直等 */ 
        if (pdTRUE == xReturn) 
            printf("Data RX: %c\r\n", r_queue); 
        else 
            printf("Data RX ERR: 0x%lx\r\n",xReturn); 
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
            printf("input byte is %c \r\n", Byte);
            xReturn = xQueueSend( Test_Queue, /* 消息队列的句柄 */ 
                                  &Byte,/* 发送的消息内容 */
                                  0 ); /* 等待时间 0 */

            if (pdPASS == xReturn)
            {
                printf("Data TX %c success\r\n", Byte);
            }

            vTaskDelay(20);/* 延时 20 个 tick */
        }
    }
}

BaseType_t TestMsgQueueEntry()
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

    Uart1Init();

    printf("get cpu stats TestMsgQueueEntry\r\n");
    xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate, /* 任务入口函数 */
                            (const char* )"AppTaskCreate",/* 任务名字 */
                            (uint16_t )1024, /* 任务栈大小 */
                            (void* )NULL,/* 任务入口函数参数 */
                            (UBaseType_t )1, /* 任务的优先级 */
                            (TaskHandle_t* )&AppTaskCreate_Handle); /* 任务控制块指针 */

    return   xReturn;
}