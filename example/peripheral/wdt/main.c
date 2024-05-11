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
 * Description:  This file is for wdt test main entry.
 *
 * Modify History:
 *  Ver      Who            Date           Changes
 * -----   ------         --------     --------------------------------------
 *  1.0   wangxiaodong    2022/8/9      first release
 *  2.0   liqiaozhong     2024/4/22     add no letter shell mode, adapt to auto-test system
 */

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "sdkconfig.h"

#include "wdt_example.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#endif

#define WDT_EXAMPLE_TASK_PRIORITY 2

void WdtExampleTaskEntry()
{
    /* example functions */
    FFreeRTOSWdtCreate();

    /* end flag */
    printf("[test_end]\r\n");
    vTaskDelete(NULL);
}

int main(void)
{
    BaseType_t ret = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */

    /* used in no-letter-shell mode */
    ret = xTaskCreate((TaskFunction_t)WdtExampleTaskEntry,    /* 任务入口函数 */
                      (const char *)"WdtExampleTaskEntry",    /* 任务名字 */
                      (uint16_t)4096,                          /* 任务栈大小 */
                      NULL,                                    /* 任务入口函数参数 */
                      (UBaseType_t)WDT_EXAMPLE_TASK_PRIORITY, /* 任务优先级 */
                      NULL);            
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    }

#ifdef CONFIG_USE_LETTER_SHELL
    ret = LSUserShellTask();
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    }
#endif

    vTaskStartScheduler(); /* 启动任务，开启调度 */
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("WDT example failed in main.c, the ret value is 0x%x. \r\n", ret);
    return -2;
}