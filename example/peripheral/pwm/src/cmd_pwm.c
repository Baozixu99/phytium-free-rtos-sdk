/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
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
    printf("pwm dead_band\r\n");
    printf("-- run pwm dead band example at controller \r\n");
    printf("pwm dual_channel\r\n");
    printf("-- run pwm dual channel at controller \r\n");
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
    if (!strcmp(argv[1], "dead_band"))
    {
        ret = FFreeRTOSPwmDeadBandTaskCreate();
    }

    if (!strcmp(argv[1], "dual_channel"))
    {
        ret = FFreeRTOSPwmDualChannelTaskCreate();
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), pwm, PwmCmdEntry, test freertos pwm driver);
#endif