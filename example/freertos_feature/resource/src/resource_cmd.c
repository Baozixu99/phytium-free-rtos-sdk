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
 * FilePath: resource_cmd.c
 * Date: 2022-06-17 10:41:45
 * LastEditTime: 2024-05-07 10:41:45
 * Description:  This file is for resource command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 * 1.1 huangjin     2024/05/07  add no letter shell mode, adapt to auto-test system
 */
#include "sdkconfig.h"
#include "FreeRTOS.h"
#include "feature_resource.h"
#include <string.h>
#include "task.h"
#include <stdio.h>
#include "strto.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"

static void ResourceTasksCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("resource mutex_cre \r\n");
    printf("-- Create mutex tasks now. \r\n");
    printf("resource gate_cre \r\n");
    printf("-- Create gatekeeper tasks now. \r\n");
}

int ResourceTasksCmd(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        ResourceTasksCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "mutex_cre"))
    {
        ret = CreateResourceTasks();
    }
    else if (!strcmp(argv[1], "gate_cre"))
    {
        ret = CreateGatekeeperTasks();
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        ResourceTasksCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), resource, ResourceTasksCmd, Resource Management test);
#endif