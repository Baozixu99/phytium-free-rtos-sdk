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
 * FilePath: cmd_gdma.c
 * Date: 2022-07-14 14:06:43
 * LastEditTime: 2022-07-14 14:06:43
 * Description:  This files is for gdma command interface
 *
 * Modify History:
 *  Ver      Who           Date         Changes
 * -----    ------       --------      --------------------------------------
 *  1.0    zhugengyu     2022/7/27     init commit
 *  2.0    liqiaozhong   2024/4/22     add no letter shell mode, adapt to auto-test system
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "strto.h"

#include "sdkconfig.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"

#include "gdma_memcpy.h"
#endif
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
#ifdef CONFIG_USE_LETTER_SHELL
static void GdmaCmdUsage()
{
    printf("Usage:\r\n");
    printf("gdma memcpy\r\n");
    printf("-- Run GDMA memcpy example\r\n");
}

static int GdmaCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        GdmaCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "memcpy"))
    {
        ret = FFreeRTOSGdmaMemcpy();
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), gdma, GdmaCmdEntry, test freertos gdma driver);
#endif