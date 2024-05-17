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


