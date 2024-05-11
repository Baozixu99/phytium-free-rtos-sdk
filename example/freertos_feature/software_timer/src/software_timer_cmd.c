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
 * FilePath: software_timer_cmd.c
 * Date: 2022-06-17 10:41:45
 * LastEditTime: 2022-06-17 10:41:45
 * Description:  This file is for software timer command interface
 *
 * Modify History:
 *  Ver      Who           Date         Changes
 * -----    ------       --------      --------------------------------------
 *  1.0   wangxiaodong   2022/8/9      first commit
 *  2.0   liqiaozhong    2024/5/7      add no letter shell mode, adapt to auto-test system
 */

#include <string.h>
#include <stdio.h>

#include "sdkconfig.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"

#include "feature_software_timer.h"

static void SoftwareTimerCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("timer create_start_example \r\n");
    printf("-- Create and starts a one-shot timer and an auto-reload timer.\r\n");
    printf("timer id_reset_example \r\n");
    printf("-- Create software timer use timer id and reset timer.\r\n");
}

int SoftwareTimerCmd(int argc, char *argv[])
{
    if (argc < 2)
    {
        SoftwareTimerCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "create_start_example"))
    {
        CreateTimerTasks();
    }
    else if (!strcmp(argv[1], "id_reset_example"))
    {
        CreateTimerResetTasks();
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        SoftwareTimerCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), timer, SoftwareTimerCmd, software timer test);
#endif