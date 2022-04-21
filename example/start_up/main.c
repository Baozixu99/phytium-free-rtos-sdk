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
 * FilePath: main.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:02:16
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */


#include "sdkconfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

void TestTask(void *param)
{
    while (1)
    {
        printf("hello world!!!\r\n");
        vTaskDelay(100);
    }
}

void HelloWorldEntry()
{
    if (xTaskCreate(TestTask, "hello", 256, NULL, 5, NULL) != pdPASS)
    {
        printf("create task failed \r\n");
    }  

    vTaskStartScheduler(); /* 启动任务，开启调度 */   
}

extern void TestTaskPriority();
extern void TestCpuStatsEntry();
extern void TestMsgQueueEntry();
extern void TestSemaphoreEntry();
void main()
{
    printf("freertos start up\r\n");

#ifdef CONFIG_DEMO_HELLO_WORLD
    HelloWorldEntry();
#endif

#ifdef CONFIG_DEMO_GET_CPU_STATS
    TestCpuStatsEntry();
#endif

#ifdef CONFIG_DEMO_MSG_QUEUE
    TestMsgQueueEntry();
#endif

#ifdef CONFIG_DEMO_SEMAPHORE
    TestSemaphoreEntry();
#endif
}
