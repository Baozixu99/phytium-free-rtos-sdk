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
 * Description:  This file is for eventgroup example that running shell task and open scheduler
 *
 * Modify History:
 *  Ver      Who           Date         Changes
 * -----    ------       --------      --------------------------------------
 *  1.0   wangxiaodong   2022/8/9      first commit
 *  2.0   liqiaozhong    2024/5/6      add no letter shell mode, adapt to auto-test system
 */

#include <stdio.h>

#include "FreeRTOS.h"

#include "sdkconfig.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#else
#include "task.h"
#include "feature_eventgroup.h"
#define EVENTGROUP_EXAMPLE_TASK_PRIORITY 2
void EventgroupExampleTaskEntry()
{
    CreateManagementTasks();
    CreateSyncTasks();

    printf("[test_end]\r\n");
    vTaskDelete(NULL);
}
#endif

int main(void)
{
    BaseType_t ret = pdPASS; /* Define a return value with a default of pdPASS */
#ifdef CONFIG_USE_LETTER_SHELL
    ret = LSUserShellTask();
#else
    /* used in no-letter-shell mode */
    ret = xTaskCreate((TaskFunction_t)EventgroupExampleTaskEntry,
                      (const char *)"EventgroupExampleTaskEntry",
                      (uint16_t)4096,
                      NULL,
                      (UBaseType_t)EVENTGROUP_EXAMPLE_TASK_PRIORITY,
                      NULL);
#endif
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    }

    vTaskStartScheduler(); /* 启动任务，开启调度 */
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("Event group example failed in main.c, the ret value is 0x%x. \r\n", ret);
    return -2;
}