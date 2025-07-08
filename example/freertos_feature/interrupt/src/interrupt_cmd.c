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
 * FilePath: interrupt_cmd.c
 * Date: 2022-06-17 10:41:45
 * LastEditTime: 2024-05-06 10:41:45
 * Description:  This file is for interrupt command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 * 1.1 huangjin     2024/05/06  add no letter shell mode, adapt to auto-test system
 */
#include "sdkconfig.h"
#include "FreeRTOS.h"
#include "feature_interrupt.h"
#include <string.h>
#include "task.h"
#include <stdio.h>
#include "strto.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"

static void CreateIntrCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("intr bin_cre \r\n");
    printf("-- Create intr binary sem tasks now.\r\n");
    printf("intr count_cre \r\n");
    printf("-- Create counting sem tasks now.\r\n");
    printf("intr queue_cre \r\n");
    printf("-- Create queue tasks now.\r\n");
}

int CreateIntrCmd(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        CreateIntrCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "bin_cre"))
    {
        ret = CreateBinarySemTasks();
    }
    else if (!strcmp(argv[1], "count_cre"))
    {
        ret = CreateCountSemTasks();
    }
    else if (!strcmp(argv[1], "queue_cre"))
    {
        ret = CreateQueueTasks();
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreateIntrCmdUsage();
    }
    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), intr, CreateIntrCmd, intr task test);
#endif