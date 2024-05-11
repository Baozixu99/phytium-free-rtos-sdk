/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: eventgroup_cmd.c
 * Date: 2022-06-17 10:42:40
 * LastEditTime: 2022-06-17 10:42:40
 * Description:  This file is for eventgroup command interface
 *
 * Modify History:
 *  Ver      Who           Date         Changes
 * -----    ------       --------      --------------------------------------
 *  1.0   wangxiaodong   2022/8/9      first commit
 *  2.0   liqiaozhong    2024/5/6      add no letter shell mode, adapt to auto-test system
 */

#include <string.h>
#include <stdio.h>

#include "sdkconfig.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"

#include "feature_eventgroup.h"

static void EventTasksCmdUsage(void)
{
    printf("Usage: \r\n");
    printf("event manage_example \r\n");
    printf("-- Create manage tasks. \r\n");
    printf("event sync_example \r\n");
    printf("-- Create sync tasks. \r\n");
}

int CreateEventCmd(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        EventTasksCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "manage_example"))
    {
        ret = CreateManagementTasks();
    }
    else if (!strcmp(argv[1], "sync_example"))
    {
        ret = CreateSyncTasks();   
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        EventTasksCmdUsage();
    }

    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), event, CreateEventCmd, event group creating test);
#endif