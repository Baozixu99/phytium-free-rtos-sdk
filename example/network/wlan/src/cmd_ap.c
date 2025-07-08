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
 * FilePath: cmd_ap.c
 * Date: 2022-07-12 09:33:12
 * LastEditTime: 2022-07-12 09:33:12
 * Description:  This file is for providing user command functions.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/8/26    first commit
 */

/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"

#include "FreeRTOS.h"
#include "wlan_common.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"
#include "wlan_ap_config.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
static void APCmdUsage()
{
    printf("Usage:\r\n");
    printf("    wlan_ap start\r\n");
    printf("        -- Start run the wireless access point\r\n");
}

static int APCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        APCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "start"))
    {
        if (pdPASS != FFreeRTOSStartAP())
        {
            return -2;
        }
    }

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), wlan_ap, APCmdEntry, wireless ap functions);

#endif

