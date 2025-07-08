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
 * FilePath: cmd_spim.c
 * Date: 2024-04-22 14:53:42
 * LastEditTime: 2024-04-24 17:46:03
 * Description:  This file is for spim_spiffs example cmd catalogue
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun   2024/4/22   first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "spim_spiffs_example.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

static void SpimSpiffsCmdUsage()
{
    printf("Usage:\r\n");
    printf("spim_spiffs read_write\r\n");
    printf("-- Demo read and write by spim_spiffs\r\n");
}

static int SpimSpiffsCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        SpimSpiffsCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "read_write"))
    {
        ret = FFreeRTOSSpimSpiffsRunWR();
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), spim_spiffs, SpimSpiffsCmdEntry, test freertos spim_spiffs);
#else
void SpimSpiffsExampleEntry(void)
{
    FFreeRTOSSpimSpiffsRunWR();

    printf("[test_end].\r\n");
    vTaskDelete(NULL);
}
#endif