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


