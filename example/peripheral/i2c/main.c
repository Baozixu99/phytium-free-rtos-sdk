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
 * Date: 2022-07-18 14:39:10
 * LastEditTime: 2022-07-18 14:39:10
 * Description:  This file is for i2c main entry.
 *
 * Modify History:
 *  Ver       Who            Date                 Changes
 * -----    ------         --------     --------------------------------------
 *  1.0    liushengming   2022/11/25             init commit
 *  1.1   zhangyan       2024/4/18     add no letter shell mode, adapt to auto-test system
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "FreeRTOS.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#else
#include "task.h"
#include "i2c_example.h"
#define I2C_EXAMPLE_TASK_PRIORITY 2
void I2cExampleTaskEntry()
{
#if defined(CONFIG_E2000D_DEMO_BOARD)||defined(CONFIG_E2000Q_DEMO_BOARD)
    FFreeRTOSI2cRtcCreate();
#elif defined(CONFIG_FIREFLY_DEMO_BOARD)
    FFreeRTOSI2cLoopbackCreate();
#endif
    printf("[test_end]\r\n");

    vTaskDelete(NULL);
}
#endif
int main(void)
{
    BaseType_t ret;

#ifdef CONFIG_USE_LETTER_SHELL
    ret = LSUserShellTask();
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    }
#else
    taskENTER_CRITICAL();                                   /*进入临界区*/
    ret = xTaskCreate((TaskFunction_t)I2cExampleTaskEntry, /* 任务入口函数 */
                      (const char *)"I2cExampleTaskEntry", /* 任务名字 */
                      (uint16_t)4096,                       /* 任务栈大小 */
                      NULL,                                 /* 任务入口函数参数 */
                      (UBaseType_t)I2C_EXAMPLE_TASK_PRIORITY,                       /* 任务的优先级 */
                      NULL);
    taskEXIT_CRITICAL(); /*退出临界区*/
#endif

    vTaskStartScheduler(); /* 启动任务，开启调度 */
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("Failed,the ret value is 0x%x. \r\n", ret);
    return 0;
}