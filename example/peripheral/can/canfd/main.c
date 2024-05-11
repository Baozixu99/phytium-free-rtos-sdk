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
 * Date: 2023-10-20 08:17:59
 * LastEditTime: 2023-10-20 08:17:59
 * Description:  This file is for CANFD example that running CANFD task、shell task and open scheduler
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2023/10/20  first commit
 * 2.0   huangjin   2024/04/25  add no letter shell mode, adapt to auto-test system
 */

#include <stdio.h>
#include "FreeRTOS.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#else
#include "task.h"
#include "canfd_example.h"

#define CANFD_EXAMPLE_TASK_PRIORITY 2

void CanfdExampleTaskEntry(void *pvParameters)
{
    /* example functions */
    FFreeRTOSCreateCanfdIntrTestTask();
    FFreeRTOSCreateCanfdPolledTestTask();
    FFreeRTOSCanfdCreateFilterTestTask();

    /* end flag */
    printf("[test_end]\r\n");
    vTaskDelete(NULL);
}
#endif


int main(void)
{
    BaseType_t ret = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

#ifdef CONFIG_USE_LETTER_SHELL
    ret = LSUserShellTask();
#else
    /* used in no-letter-shell mode */
    ret = xTaskCreate((TaskFunction_t)CanfdExampleTaskEntry,  /* 任务入口函数 */
                          (const char *)"CanfdExampleTaskEntry",/* 任务名字 */
                          (uint16_t)4096,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)CANFD_EXAMPLE_TASK_PRIORITY,  /* 任务的优先级 */
                          NULL);
#endif
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    }

    vTaskStartScheduler(); /* 启动任务，开启调度 */
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("CANFD example failed in main.c, the ret value is 0x%x. \r\n", ret);
    return 0;
}
