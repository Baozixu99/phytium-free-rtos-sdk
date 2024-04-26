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
 * FilePath: cmd_pwm.c
 * Date: 2022-07-14 14:06:43
 * LastEditTime: 2022-07-14 14:06:43
 * Description:  This files is for pwm command interface
 *
 * Modify History:
 *  Ver      Who           Date         Changes
 * -----    ------       --------      --------------------------------------
 *  1.0    zhangyan     2024/4/23     init commit
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "strto.h"

#include "sdkconfig.h"
#include "pwm_example.h"
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"

static void PwmCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("pwm single_channel\r\n");
    printf("-- run pwm single channel at controller \r\n");
#ifdef CONFIG_FIREFLY_DEMO_BOARD
    printf("pwm dead_band\r\n");
    printf("-- run pwm dead band example at controller \r\n");
    printf("pwm dual_channel\r\n");
    printf("-- run pwm dual channel at controller \r\n");
#endif
}

static int PwmCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        PwmCmdUsage();
        return -1;
    }
    /* parser example input args and run example */
    if (!strcmp(argv[1], "single_channel"))
    {
        ret = FFreeRTOSPwmSingleChannelTaskCreate();
    }
#ifdef CONFIG_FIREFLY_DEMO_BOARD
    if (!strcmp(argv[1], "dead_band"))
    {
        ret = FFreeRTOSPwmDeadBandTaskCreate();
    }

    if (!strcmp(argv[1], "dual_channel"))
    {
        ret = FFreeRTOSPwmDualChannelTaskCreate();
    }
#endif

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), pwm, PwmCmdEntry, test freertos pwm driver);
#endif