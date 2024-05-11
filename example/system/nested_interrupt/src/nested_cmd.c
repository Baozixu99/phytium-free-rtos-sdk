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
 * FilePath: nested_cmd.c
 * Date: 2022-06-17 10:41:45
 * LastEditTime: 2022-06-17 10:41:45
 * Description:  This file is for nested interrupt command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2023/02/25 first commit
 * 1.1  zhangyan     2024/4/29    add no letter shell mode, adapt to auto-test system
 */
#include <string.h>
#include <stdio.h>
#include "nested_interrupt.h"
#include "sdkconfig.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
static void CreateNestIntrCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" nest cre \r\n");
    printf("    -- Create nest tasks now.\r\n");
}

int CreateNestIntrCmd(int argc, char *argv[])
{
    if (argc < 2)
    {
        CreateNestIntrCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "cre"))
    {
        FFreeRTOSNestedIntrTaskCreate();
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreateNestIntrCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), nest, CreateNestIntrCmd, nest interrupt task test);
#endif


