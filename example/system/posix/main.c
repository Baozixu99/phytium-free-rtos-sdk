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
 * LastEditTime: 2022-03-21 17:02:53
 * Description:  This file is for providing a template main.c file when creating new freertos examples.
 *
 * Modify History:
 *  Ver   Who            Date         Changes
 * ----- ------        --------    --------------------------------------
 *  1.0  wangxiaodong  2022/11/1    init commit
 */
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "ftypes.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#else
#include "task.h"
#include "posix_example.h"
#define FREERTOS_POSIX_EXAMPLE_TASK_PRIORITY 2
void FFreeRTOSPosixExampleTaskEntry()
{
    CreatePOSIXDemoTasks();

    printf("[test_end]\r\n");

    vTaskDelete(NULL);
}
#endif

int main()
{
    BaseType_t xReturn = pdPASS;

#ifdef CONFIG_USE_LETTER_SHELL
    xReturn = LSUserShellTask();
#else                                 
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSPosixExampleTaskEntry, /* 任务入口函数 */
                      (const char *)"FFreeRTOSPosixExampleTaskEntry", /* 任务名字 */
                      4096,                       /* 任务栈大小 */
                      NULL,                                 /* 任务入口函数参数 */
                      (UBaseType_t)FREERTOS_POSIX_EXAMPLE_TASK_PRIORITY,                       /* 任务的优先级 */
                      NULL);
#endif
    if (xReturn != pdPASS)
    {
        goto FAIL_EXIT;
    }

    vTaskStartScheduler(); /* 启动任务，开启调度 */

    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("Failed,the xReturn value is 0x%x. \r\n", xReturn);
    return 0;
}