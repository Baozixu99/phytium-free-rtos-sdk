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


