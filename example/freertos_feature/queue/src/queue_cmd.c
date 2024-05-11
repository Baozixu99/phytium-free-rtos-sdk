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
 * FilePath: queue_cmd.c
 * Date: 2022-06-17 10:41:45
 * LastEditTime: 2022-06-17 10:41:45
 * Description:  This file is for queue command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 */
#include <string.h>
#include <stdio.h>
#include "feature_queue.h"
#include "sdkconfig.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"

static void CreateQueueCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" queue int_cre \r\n");
    printf("    -- Create int queue send and receive tasks now.\r\n");
    printf(" queue struct_cre \r\n");
    printf("    -- Create struct queue send and receive tasks now.\r\n");
    printf(" queue set_cre \r\n");
    printf("    -- Use queue set function, create send and receive tasks now.\r\n");
}

int CreateQueueCmd(int argc, char *argv[])
{
    if (argc < 2)
    {
        CreateQueueCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "int_cre"))
    {
         CreateIntTasks();
    }
    else if (!strcmp(argv[1], "struct_cre"))
    {
        CreateStructTasks();
    }
    else if (!strcmp(argv[1], "set_cre"))
    {
        CreateQueueSetTasks();
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreateQueueCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), queue, CreateQueueCmd, queue task creating test);
#endif


