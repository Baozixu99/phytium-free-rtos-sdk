/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: posix_example_cmd.c
 * Date: 2023-10-12 10:41:45
 * LastEditTime: 2023-10-12 10:41:45
 * Description:  This file is for freertos posix command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2023/06/25 first commit
 */

#include <string.h>
#include <stdio.h>
#include "posix_example.h"
#include "sdkconfig.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
static void CreatePosixCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" posix demo \r\n");
    printf("    -- Create posix test now.\r\n");
}

int CreatePosixCmd(int argc, char *argv[])
{
    if (argc < 2)
    {
        CreatePosixCmdUsage();
        return -1;
    }
    if (!strcmp(argv[1], "demo"))
    {
        CreatePOSIXDemoTasks();
    }

    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreatePosixCmdUsage();
    }
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), posix, CreatePosixCmd, posix task test);
#endif
