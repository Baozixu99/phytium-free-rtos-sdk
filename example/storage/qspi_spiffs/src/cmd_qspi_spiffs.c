/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: cmd_qspi_spiffs.c
 * Date: 2024-04-25 14:06:43
 * LastEditTime: 2024-04-25 14:06:43
 * Description:  This files is for qspi spiffs command interface
 *
 * Modify History:
 *  Ver      Who           Date         Changes
 * -----    ------       --------      --------------------------------------
 *  1.0     huangjin     2024/04/25    add no letter shell mode, adapt to auto-test system
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "strto.h"

#include "sdkconfig.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"

#include "qspi_spiffs_example.h"
#endif
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
#ifdef CONFIG_USE_LETTER_SHELL
static void QspiSpiffsCmdUsage()
{
    printf("Usage:\r\n");
    printf("qspi_spiffs write_read\r\n");
    printf("-- Demo write and read by qspi_spiffs\r\n");
}

static int QspiSpiffsCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        QspiSpiffsCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "write_read"))
    {
        ret = FFreeRTOSQspiSpiffsCreate();
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), qspi_spiffs, QspiSpiffsCmdEntry, test freertos qspi_spiffs);
#endif