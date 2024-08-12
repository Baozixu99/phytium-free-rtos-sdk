/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * Date: 2022-06-17 08:17:59
 * LastEditTime: 2022-06-17 08:17:59
 * Description:  This file is for queue example that running shell task and open scheduler
 *
 * Modify History:
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 * 1.0 wangxiaodong 2022/06/20  first commit
 */

#include <stdio.h>
#include "FreeRTOS.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#else
#include "task.h"
#include "feature_queue.h"
void QueueExampleTaskEntry(void)
{
    CreateIntTasks();   
    CreateStructTasks();    
    CreateQueueSetTasks();  

    printf("[test_end]\r\n");
    vTaskDelete(NULL);
}
#endif

int main(void)
{
    BaseType_t ret;

#ifdef CONFIG_USE_LETTER_SHELL
    ret = LSUserShellTask() ;
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    }
#else

    taskENTER_CRITICAL(); /*进入临界区*/
    ret = xTaskCreate((TaskFunction_t)QueueExampleTaskEntry,  /* 任务入口函数 */
                          (const char *)"QueueExampleTaskEntry",/* 任务名字 */
                          4096,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)8,  /* 任务的优先级 */
                          NULL);
    taskEXIT_CRITICAL(); /*退出临界区*/

#endif

    vTaskStartScheduler(); /* 启动任务，开启调度 */
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("Failed 0x%x. \r\n", ret);
    return 0;
}
