/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * FilePath: main.c
 * Date: 2022-08-23 17:24:03
 * LastEditTime: 2022-08-23 17:24:03
 * Description:  This file is for timer_tacho main entry.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2022/11/25   init
 * 2.0  wangzq      2024/4/29    add no letter shell mode, adapt to auto-test system
 */

#include <stdio.h>

#include "FreeRTOS.h"

#include "sdkconfig.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#else
#include "task.h"
#include "timer_tacho_example.h"

#define TIMER_TACHO_EXAMPLE_TASK_PRIORITY 2

void TimerTachoExampleTaskEntry()
{
    /* example functions */
    FFreeRTOSTimerTachoCreate();

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
    ret = xTaskCreate((TaskFunction_t)TimerTachoExampleTaskEntry,    /* 任务入口函数 */
                      (const char *)"TimerTachoExampleTaskEntry",    /* 任务名字 */
                      4096,                          /* 任务栈大小 */
                      NULL,                                    /* 任务入口函数参数 */
                      (UBaseType_t)TIMER_TACHO_EXAMPLE_TASK_PRIORITY, /* 任务优先级 */
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
    printf("Timer_tacho example failed in main.c, the ret value is 0x%x. \r\n", ret);
    return 0;
}
