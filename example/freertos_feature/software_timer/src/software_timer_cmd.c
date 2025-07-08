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