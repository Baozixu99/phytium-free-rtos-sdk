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
 * FilePath: exception_cmd.c
 * Date: 2022-06-17 10:41:45
 * LastEditTime: 2022-06-17 10:41:45
 * Description:  This file is for exception command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2023/02/25 first commit
 */

#include <string.h>
#include <stdio.h>
#include "exception_test.h"
#include "sdkconfig.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"

static void CreateExceptionCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" exception undef \r\n");
    printf("    --  a64--Synchronous-Invalid instructions, a32--UNDEFINED instruction.\r\n");
    printf(" exception access \r\n");
    printf("    --  a64--Synchronous-Memory access, a32--Data Abort.\r\n");
    printf(" exception abort \r\n");
    printf("    --  a64--SError, a32--Data Abort.\r\n");
}

int CreateExceptionCmd(int argc, char *argv[])
{
    if (argc < 2)
    {
        CreateExceptionCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "undef"))
    {
        FExcOpsUndefInstr();
    }
    else if (!strcmp(argv[1], "access"))
    {
        FExcOpsInvalidMemAccess();
    }
    else if (!strcmp(argv[1], "abort"))
    {
        FExcOpsDataAbort();
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreateExceptionCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), exception, CreateExceptionCmd, exception test);
#endif


