/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * Date: 2023-02-22 08:17:59
 * LastEditTime: 2023-3-1 08:17:59
 * Description:  This file is for nested interrupt test main entry.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 * 1.0  wangxiaodong 2023/2/23	  first release
 * 1.1  zhangyan     2024/4/29    add no letter shell mode, adapt to auto-test system
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "FreeRTOS.h"
#include "task.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#else
#include "task.h"
#include "nested_interrupt.h"
#define NESTED_EXAMPLE_TASK_PRIORITY 2

void NestedExampleTaskEntry()
{
    FFreeRTOSNestedIntrTaskCreate();

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
    xReturn = xTaskCreate((TaskFunction_t)NestedExampleTaskEntry, /* 任务入口函数 */
                      (const char *)"NestedExampleTaskEntry", /* 任务名字 */
                      4096,                       /* 任务栈大小 */
                      NULL,                                 /* 任务入口函数参数 */
                      (UBaseType_t)NESTED_EXAMPLE_TASK_PRIORITY,                       /* 任务的优先级 */
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
