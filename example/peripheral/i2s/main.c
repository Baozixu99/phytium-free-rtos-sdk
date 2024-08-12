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
 * Date: 2024-02-29 13:22:40
 * LastEditTime: 2024-02-29 15:40:40
 * Description:  This file is for function of main
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wzq         2024/02/29  first commit
 * 1.1  wzq         2024/04/25  add shell support
 */


#include <stdio.h>

#include "FreeRTOS.h"

#include "sdkconfig.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#else
#include "task.h"
#include "i2s_example.h"

#define I2S_EXAMPLE_TASK_PRIORITY 2

void I2sExampleTaskEntry()
{
    /* example functions */
    FFreeRTOSRunI2sExample();

    /* end flag */
    printf("[test_end]\r\n");
    vTaskDelete(NULL);
}
#endif

int main(void)
{
    BaseType_t ret = pdPASS;

#ifdef CONFIG_USE_LETTER_SHELL
    ret = LSUserShellTask();
#else
    /* used in no-letter-shell mode */
    ret = xTaskCreate((TaskFunction_t)I2sExampleTaskEntry,    /* 任务入口函数 */
                      (const char *)"I2sExampleTaskEntry",    /* 任务名字 */
                      4096,                          /* 任务栈大小 */
                      NULL,                                    /* 任务入口函数参数 */
                      (UBaseType_t)I2S_EXAMPLE_TASK_PRIORITY, /* 任务优先级 */
                      NULL);                                   /* 任务句柄 */
#endif

    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    }

    /* 启动任务，开启调度 */
    vTaskStartScheduler();
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("I2s example failed in main.c, the ret value is 0x%x. \r\n", ret);
    return 0;
}