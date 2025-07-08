/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * Date: 2025-04-21 14:39:10
 * LastEditTime: 2025-04-21 14:39:10
 * Description:  This file is for i2c main entry.
 *
 * Modify History:
 *  Ver       Who            Date              Changes
 * -----    ------         --------          --------------------------------------
 *  1.0    zhangyan        2025/04/21         init commit
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "FreeRTOS.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#else
#include "task.h"
#include "i2c_msg_example.h"
#define I2C_MSG_EXAMPLE_TASK_PRIORITY 2
void I2cMsgWriteThenReadExampleTaskEntry()
{
#if defined(CONFIG_PD2408_TEST_B_BOARD)
    FFreeRTOSI2cMsgWriteThenReadCreate();
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
    ret = xTaskCreate((TaskFunction_t)I2cMsgWriteThenReadExampleTaskEntry, /* 任务入口函数 */
                      (const char *)"I2cMsgWriteThenReadExampleTaskEntry", /* 任务名字 */
                      4096,                       /* 任务栈大小 */
                      NULL,                                 /* 任务入口函数参数 */
                      (UBaseType_t)I2C_MSG_EXAMPLE_TASK_PRIORITY,                       /* 任务的优先级 */
                      NULL);
    taskEXIT_CRITICAL(); /*退出临界区*/
#endif

    vTaskStartScheduler(); /* 启动任务，开启调度 */
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("Failed,the ret value is 0x%x. \r\n", ret);
    return 0;
}