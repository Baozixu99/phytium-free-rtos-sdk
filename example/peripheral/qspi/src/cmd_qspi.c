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
 * FilePath: cmd_qspi.c
 * Date: 2023-11-16 10:41:45
 * LastEditTime: 2023-11-16 10:41:45
 * Description:  This file is for qspi example command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2023/11/16  first commit
 */
#include "qspi_example.h"
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "sdkconfig.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
static void CreateTasksCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("qspi check \r\n");
    printf("-- run qspi flash connection check example at controller \r\n");
    printf("qspi polled\r\n");
    printf("-- run qspi flash polled example at controller\r\n");
    printf("qspi indirect \r\n");
    printf("-- run qspi flash indirect example at controller\r\n");
    printf("qspi dual_flash_test \r\n");
    printf("-- run qspi dual flash stack example at controller\r\n");
}

int CreateTasksCmd(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        CreateTasksCmdUsage();
        return -1;
    }
    else if (!strcmp(argv[1], "check"))
    {
        ret = FFreeRTOSQspiCheckTaskCreate();
    }
    else if (!strcmp(argv[1], "polled"))
    {
        ret = FFreeRTOSQspiPolledTaskCreate();
    }
    else if (!strcmp(argv[1], "indirect"))
    {
        ret = FFreeRTOSQspiIndirectTaskCreate();
    }
    else if (!strcmp(argv[1], "dual_flash_test"))
    {
        ret = FFreeRTOSQspiDualFlashTaskCreate();
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreateTasksCmdUsage();
    }

    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), qspi, CreateTasksCmd, qspi creating test);
#endif
