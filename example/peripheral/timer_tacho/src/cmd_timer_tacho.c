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
 * FilePath: cmd_timer_tacho.c
 * Date: 2022-06-28 14:42:53
 * LastEditTime: 2022-06-28 14:42:53
 * Description:  This file is for timer tacho shell command implmentation.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  2.0  wangzq     2024/4/22     add no letter shell mode, adapt to auto-test system
 */
/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"

#include "FreeRTOS.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"
#include "timer_tacho_example.h"
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/

static void TimerTachoCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("timer tacho\r\n");
    printf("-- Demo to test timer and tacho \r\n");
}
static int TimerTachoCmdEntry(int argc, char *argv[])
{
    int ret = 0;
   
    if (argc < 2)
    {
        TimerTachoCmdUsage();
        return -1;
    }
    if (!strcmp(argv[1], "tacho"))
    {
        ret = FFreeRTOSTimerTachoCreate();
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), timer, TimerTachoCmdEntry, test freertos timer_tacho driver);

#endif /* CONFIG_USE_LETTER_SHELL */

