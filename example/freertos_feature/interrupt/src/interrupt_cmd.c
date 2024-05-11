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