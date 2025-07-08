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
 * Date: 2022-06-17 08:17:59
 * LastEditTime: 2024-04-19 08:17:59
 * Description:  This file is for DDMA example that running shell task and open scheduler.
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 zhugengyu    2022/08/26  first commit
 * 2.0 liyilun      2024/4/19   add no letter shell mode, adapt to auto test system
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include"ddma_spi_loopback.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
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

    taskENTER_CRITICAL();
    ret = xTaskCreate((TaskFunction_t)DdmaTasksEntry,
                        (const char *)"DdmaTaskEntry",
                        4096,
                        NULL,
                        (UBaseType_t)2,
                        NULL);
    taskEXIT_CRITICAL();
#endif
    vTaskStartScheduler(); /* 启动任务，开启调度 */
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("failed 0x%x \r\n", ret);
    return 0;
}
