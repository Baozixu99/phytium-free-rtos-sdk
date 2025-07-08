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
 * FilePath: cmd_ps.c
 * Date: 2022-02-25 08:34:53
 * LastEditTime: 2022-02-25 08:34:53
 * Description:  This file is for the ps command functions
 *
 * Modify History:
*  Ver     Who          Date         Changes
 * -----  ------      --------    --------------------------------------
 * 1.0  wangxiaodong  2022/11/24  first release
 */
#include "FreeRTOS.h"
#include "task.h"
#include "../src/shell.h"
#include <stdio.h>
#include "string.h"

#define PS_LENGTH 10240

static uint8_t CPU_RunInfo[PS_LENGTH] = {0}; //保存任务运行时间信息

static int DisplayTaskStats(int argc, char *argv[])
{
    memset(CPU_RunInfo, 0, PS_LENGTH); /*信息缓冲区清零*/
    vTaskList((char *)&CPU_RunInfo); //获取任务运行时间信息

    printf("---------------------------------------------\r\n");
    printf("task_name\t\t  task_state  priority  stack  task_num\r\n");
    printf("%s", CPU_RunInfo);
    printf("---------------------------------------------\r\n");

    memset(CPU_RunInfo, 0, PS_LENGTH); //信息缓冲区清零

    vTaskGetRunTimeStats((char *)&CPU_RunInfo);

    printf("task_name\t\t\trun_time_count\tusage_rate\r\n");
    printf("%s", CPU_RunInfo);
    printf("---------------------------------------------\r\n\n");

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), ps, DisplayTaskStats, View the running status of the current task);