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
 * LastEditTime: 2022-06-17 08:17:59
 * Description:  This file is for queue example that running shell task and open scheduler
 *
 * Modify History:
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 * 1.0 wangxiaodong 2022/06/20  first commit
 */

#include <stdio.h>
#include "FreeRTOS.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#else
#include "task.h"
#include "feature_queue.h"
void QueueExampleTaskEntry(void)
{
    CreateIntTasks();   
    CreateStructTasks();    
    CreateQueueSetTasks();  

    printf("[test_end]\r\n");
    vTaskDelete(NULL);
}
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

    taskENTER_CRITICAL(); /*进入临界区*/
    ret = xTaskCreate((TaskFunction_t)QueueExampleTaskEntry,  /* 任务入口函数 */
                          (const char *)"QueueExampleTaskEntry",/* 任务名字 */
                          4096,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)8,  /* 任务的优先级 */
                          NULL);
    taskEXIT_CRITICAL(); /*退出临界区*/

#endif

    vTaskStartScheduler(); /* 启动任务，开启调度 */
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("Failed 0x%x. \r\n", ret);
    return 0;
}
