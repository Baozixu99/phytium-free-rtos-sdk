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
 * FilePath: cmd_sleep.c
 * Date: 2022-02-24 18:24:53
 * LastEditTime: 2022-03-21 17:04:10
 * Description:  This file is for the sleep command functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/3/25   first release
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/shell.h"
#include "fio.h"
#include "ftypes.h"
#include "fsleep.h"

static void SleepCmdUsage()
{
    printf("usage:\r\n");
    printf("    sleep [-s | -m | -u] [num] \r\n");
    printf("         sleep for num of seconds (-s), mill-seconds(-m), micro-seconds (-u)\r\n");
}

static int SleepCmdEntry(int argc, char *argv[])
{
    u32 time = 0;

    if (argc < 3)
    {
        SleepCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "-s"))
    {
        time = strtoul(argv[2], NULL, 0);
        fsleep_seconds(time);
    }
    else if (!strcmp(argv[1], "-m"))
    {
        time = strtoul(argv[2], NULL, 0);
        fsleep_millisec(time);
    }
    else if (!strcmp(argv[1], "-u"))
    {
        time = strtoul(argv[2], NULL, 0);
        fsleep_microsec(time);
    }
    else
    {
        return -2;
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), sleep, SleepCmdEntry, blocking sleep for a period of time);