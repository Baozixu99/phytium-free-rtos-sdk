/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-06-29 17:57:10
 * @LastEditTime: 2021-07-05 14:26:47
 * @Description:  This files is for 
 * 
 * @Modify History: 
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
