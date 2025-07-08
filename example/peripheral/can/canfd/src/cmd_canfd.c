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
 * FilePath: cmd_canfd.c
 * Date: 2023-10-20 10:41:45
 * LastEditTime: 2023-10-20 12:41:45
 * Description:  This file is for canfd example command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2023/10/20   first commit
 * 2.0   huangjin   2024/04/25   add no letter shell mode, adapt to auto-test system
 */
#include "sdkconfig.h"
#include "FreeRTOS.h"
#include "canfd_example.h"
#include <string.h>
#include "task.h"
#include <stdio.h>
#include "strto.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"
static void CreateTasksCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("canfd intr \r\n");
    printf("-- Create canfd interrupt test example now. \r\n");
    printf("canfd polled \r\n");
    printf("-- Create canfd polled test example now. \r\n");
#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD)
    printf("canfd filter \r\n");
    printf("-- Create canfd filter test example now. \r\n");
#endif
}

int CanfdCmdEntry(int argc, char *argv[])
{
    if (argc < 2)
    {
        CreateTasksCmdUsage();
        return -1;
    }
    else if (!strcmp(argv[1], "intr"))
    {
        FFreeRTOSCreateCanfdIntrTestTask();
    }
    else if (!strcmp(argv[1], "polled"))
    {
        FFreeRTOSCreateCanfdPolledTestTask();
    }
#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD)
    else if (!strcmp(argv[1], "filter"))
    {
        FFreeRTOSCanfdCreateFilterTestTask();
    }
#endif
    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreateTasksCmdUsage();
    }
    
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), canfd, CanfdCmdEntry, test freertos canfd driver);
#endif