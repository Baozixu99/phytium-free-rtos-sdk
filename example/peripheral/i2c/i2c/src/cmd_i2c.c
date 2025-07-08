/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: cmd_i2c.c
 * Date: 2023-09-25 14:42:53
 * LastEditTime: 2023-09-25 14:42:53
 * Description:  This file is for i2c shell command implmentation.
 *
 * Modify History:
 *  Ver       Who            Date                 Changes
 * -----    ------         --------     --------------------------------------
 *  1.0    liushengming   2023/09/25             init commit
 */
/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"
#include "FreeRTOS.h"
#include "i2c_example.h"

/************************** Function Prototypes ******************************/

/*****************************************************************************/
#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
static void FI2cExampleUsage()
{
    printf("Usage:\r\n");
#if defined(CONFIG_E2000D_DEMO_BOARD)||defined(CONFIG_E2000Q_DEMO_BOARD)
    printf("    i2c rtc\r\n");
    printf("        -- E2000 demo board set time and read it.\r\n");
#endif
#if defined(CONFIG_PHYTIUMPI_FIREFLY_BOARD)
    printf("    i2c ms_example\r\n");
    printf("        -- firefly board,Two i2c controllers are used for master-slave communication.\r\n");
#endif
}

static int I2cCmdEntry(int argc, char *argv[])
{
    int ret = 0;
    if (argc < 2)
    {
        FI2cExampleUsage();
        return -1;
    }
#if defined(CONFIG_E2000D_DEMO_BOARD)||defined(CONFIG_E2000Q_DEMO_BOARD)
    else if (!strcmp(argv[1], "rtc"))
    {
        ret = FFreeRTOSI2cRtcCreate();
        if (ret != pdPASS)
        {
            printf("FFreeRTOSI2cRtcCreate error :0x%x!\n",ret);
            return ret;
        }
    }
#endif
#if defined(CONFIG_PHYTIUMPI_FIREFLY_BOARD)
    else if (!strcmp(argv[1], "ms_example"))
    {
        ret = FFreeRTOSI2cLoopbackCreate();
        if (ret != pdPASS)
        {
            printf("FFreeRTOSI2cLoopbackCreate error :0x%x!\n",ret);
            return ret;
        }
    }
#endif
    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), i2c, I2cCmdEntry, test freertos i2c driver);
#endif
