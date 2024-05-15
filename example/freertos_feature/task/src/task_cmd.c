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
 * FilePath: task_cmd.c
 * Date: 2022-06-17 10:41:45
 * LastEditTime: 2024-05-10 10:41:45
 * Description:  This file is for task example command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 * 2.0 liyilun      2024/05/10  add no letter shell mode, adapt to auto test system
 */

#include "feature_task.h"
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"

static void CreateTasksCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" task cre \r\n");
    printf("    -- Create tasks now. \r\n");
    printf(" task para_cre \r\n");
    printf("    -- Create parameter test tasks now. \r\n");
    printf(" task pri_cre \r\n");
    printf("    -- Create priority test tasks now. \r\n");
    printf(" task blo_cre \r\n");
    printf("    -- Create block state test tasks now. \r\n");
    printf(" task dn_cre \r\n");
    printf("    -- Create task until test tasks now. \r\n");
    printf(" task bn_cre \r\n");
    printf("    -- Create task combining blocking non blocking tasks now. \r\n");
    printf(" task idle_cre \r\n");
    printf("    -- Create task idle tasks now. \r\n");
    printf(" task cha_cre \r\n");
    printf("    -- Create task change priority tasks now. \r\n");
}


int CreateTasksCmd(int argc, char *argv[])
{

    if (argc < 2)
    {
        CreateTasksCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "cre"))
    {
        CreateTasks();
    }
    else if (!strcmp(argv[1], "para_cre"))
    {
        CreateTasksForParamterTest();
    }
    else if (!strcmp(argv[1], "pri_cre"))
    {
        CreateTasksForPriorityTest();
    }
    else if (!strcmp(argv[1], "blo_cre"))
    {
        CreateTasksForBlockTest();
    }
    else if (!strcmp(argv[1], "dn_cre"))
    {
        CreateTasksForDelayUntilTest();
    }
    else if (!strcmp(argv[1], "bn_cre"))
    {
        CreateTasksForBlockingOrNone();
    }
    else if (!strcmp(argv[1], "idle_cre"))
    {
        CreateTasksForIdleTask();
    }
    else if (!strcmp(argv[1], "cha_cre"))
    {
        CreateTasksForChangePriorityTest();
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreateTasksCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), task, CreateTasksCmd, task creating test);

#else

void TasksExampleEntry(void)
{
    CreateTasks();
    CreateTasksForParamterTest();
    CreateTasksForPriorityTest();
    CreateTasksForBlockTest();
    CreateTasksForDelayUntilTest();
    CreateTasksForBlockingOrNone();
    CreateTasksForIdleTask();
    CreateTasksForChangePriorityTest();
    printf("[test_end].\r\n");
    vTaskDelete(NULL);

}

#endif
