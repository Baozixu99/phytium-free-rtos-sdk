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