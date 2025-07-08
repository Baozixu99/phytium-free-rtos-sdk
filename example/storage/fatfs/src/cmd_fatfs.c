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
 * FilePath: cmd_fatfs.c
 * Date: 2024-04-25 14:53:42
 * LastEditTime: 2024-04-26 17:46:03
 * Description:  This file is for fatfs example cmd catalogue
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun   2024/4/25   first release
 */

/***************************** Include Files *********************************/
#include<stdio.h>
#include<string.h>

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif
#include "fatfs_examples.h"
#include "task.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

static void FatfsCmdUsage()
{
    printf("Usage:\r\n");
    printf("fatfs test\r\n");
    printf("-- test fatfs on different peripherals\r\n");
}

static int FatfsCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        FatfsCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "test"))
    {
        ret = FFreeRTOSFatfsTest();
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), fatfs, FatfsCmdEntry, test freertos fatfs);

#else

void FatfsExampleEntry(void)
{
    FFreeRTOSFatfsTest();
    printf("[test_end].\r\n");
    vTaskDelete(NULL);
}

#endif


