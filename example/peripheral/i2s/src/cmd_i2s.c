/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: cmd_i2s.c
 * Date: 2024-02-29 13:22:40
 * LastEditTime: 2024-02-29 15:40:40
 * Description:  This file is for i2s shell command implmentation.
 *
 * Modify History:
 *  Ver       Who            Date                 Changes
 * -----    ------         --------     --------------------------------------
 * 1.0      Wangzq           2024/02/29        first commit
 * 1.1      wzq              2024/04/25        add shell support
 */
/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"

#include "FreeRTOS.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"
#include "i2s_example.h"
/************************** Function Prototypes ******************************/

static void FI2sExampleUsage()
{
    printf("Usage:\r\n");
    printf("i2s example\r\n");
    printf("-- demo to test i2s transfer.\r\n");
}

static int I2sCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        FI2sExampleUsage();
        return -1;
    }
    else if (!strcmp(argv[1], "example"))
    {
        ret = FFreeRTOSRunI2sExample();

        return ret;

    }
    

    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), i2s, I2sCmdEntry, test freertos i2s driver);

#endif /* CONFIG_USE_LETTER_SHELL */