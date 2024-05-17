/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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