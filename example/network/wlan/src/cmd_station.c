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
 * FilePath: cmd_station.c
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
#include "task.h"
#include "wlan_station_scan.h"
#include "wlan_common.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"
#include "wlan_station_connect.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
static void WlanCmdUsage()
{
    printf("Usage:\r\n");
    printf("    wlan_station scan\r\n");
    printf("        -- Scan nearby wlan station\r\n");
    printf("    wlan_station join [ssid] [password] [remote-ip]\r\n");
    printf("        -- Join a wlan station in scanned station list\r\n");
    printf("    wlan_station ping [remote-ip]\r\n");
    printf("        -- Ping remote-ip after join a wlan station\r\n");
}

static int WlanCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        WlanCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "scan"))
    {
        if (pdPASS != FFreeRTOSWlanStationScanTaskCreate())
        {
            return -2;
        }
    }
    else if (!strcmp(argv[1], "join"))
    {
        if (argc < 4)
        {
            return -3;
        }

        if (pdPASS != FFreeRTOSWlanStationConnectInit(argv[2], argv[3]))
        {
            return -2;
        }
    }
    else if (!strcmp(argv[1], "ping"))
    {
        if (argc < 3)
        {
            return -3;
        }
    
        if (pdPASS != FFreeRTOSWlanStationPing(argv[2]))
        {
            return -2;
        }
    }

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), wlan_station, WlanCmdEntry, wlan functions);

#endif
