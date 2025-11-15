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
 * FilePath: cmd_echo.c
 * Date: 2022-02-10 14:53:43
 * LastEditTime: 2022-02-25 11:47:13
 * Description:  This file is for echo command implmentation
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2021-10-18   init commit
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/shell.h"
#include "fio.h"
#include "ftypes.h"

static void EchoCmdUsage()
{
    printf("usage:\r\n");
    printf("    echo [str] [str] \r\n");
    printf("         printf to shell\r\n");
}

static int EchoCmdEntry(int argc, char *argv[])
{
    int loop;

    if (argc < 2)
    {
        EchoCmdUsage();
        return -1;
    }

    for (loop = 1; loop < argc; loop++)
    {
        printf("%s", argv[loop]);
    }

    printf("\r\n");
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), echo, EchoCmdEntry,
                 printf string to shell prompt);