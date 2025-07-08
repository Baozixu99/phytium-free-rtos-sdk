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

