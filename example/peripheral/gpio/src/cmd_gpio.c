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
 * FilePath: cmd_gpio.c
 * Date: 2022-06-28 14:42:53
 * LastEditTime: 2022-06-28 14:42:53
 * Description:  This file is for gpio shell command implmentation.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/8/26    init commit
 *  2.0  wangzq     2024/4/22     add no letter shell mode, adapt to auto-test system
 *  3.0  zhugengyu  2024/5/10    rework gpio example
 */
/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"

#include "FreeRTOS.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"
#include "gpio_intr_example.h"
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/

static void GpioCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("gpio io-irq\r\n");
    printf("-- Demo to test gpio interrupt\r\n");
}
static int GpioCmdEntry(int argc, char *argv[])
{
    int ret = 0;
   
    if (argc < 2)
    {
        GpioCmdUsage();
        return -1;
    }
    if (!strcmp(argv[1], "io-irq"))
    {

        ret = FFreeRTOSRunGpioIrq();
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), gpio, GpioCmdEntry, test freertos gpio driver);

#endif /* CONFIG_USE_LETTER_SHELL */

