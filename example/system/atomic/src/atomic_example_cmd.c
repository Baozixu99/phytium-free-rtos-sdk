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
 * FilePath: atomic_example_cmd.c
 * Date: 2022-06-17 10:41:45
 * LastEditTime: 2022-06-17 10:41:45
 * Description:  This file is for atomic command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2023/06/25 first commit
 * 1.1  zhangyan     2024/4/29    add no letter shell mode, adapt to auto-test system
 */

#include <string.h>
#include <stdio.h>
#include "atomic_example.h"
#include "sdkconfig.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
static void CreateAtomicCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" atomic cre \r\n");
    printf("    -- Create atomic test task now.\r\n");
}

int CreateAtomicCmd(int argc, char *argv[])
{
    if (argc < 2)
    {
        CreateAtomicCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "cre"))
    {
        FFreeRTOSAtomicTaskCreate();
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreateAtomicCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), atomic, CreateAtomicCmd, atomic task test);
#endif
