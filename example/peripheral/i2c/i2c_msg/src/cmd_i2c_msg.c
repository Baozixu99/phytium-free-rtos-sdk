/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: cmd_i2c_msg.c
 * Date: 2025-04-21 14:42:53
 * LastEditTime: 2025-04-21 14:42:53
 * Description:  This file is for i2c msg shell command implmentation.
 *
 * Modify History:
 *  Ver       Who            Date              Changes
 * -----    ------         --------          --------------------------------------
 *  1.0    zhangyan        2025/04/21         init commit
 */

/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"
#include "FreeRTOS.h"
#include "i2c_msg_example.h"

/************************** Function Prototypes ******************************/

/*****************************************************************************/
#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
static void FI2cMsgExampleUsage()
{
    printf("Usage:\r\n");
#if defined(CONFIG_PD2408_TEST_B_BOARD)
    printf("    i2c msg_example\r\n");
    printf("        -- firefly board,Two i2c controllers are used for master-slave communication.\r\n");
#endif
}

static int I2cMsgCmdEntry(int argc, char *argv[])
{
    int ret = 0;
    if (argc < 2)
    {
        FI2cMsgExampleUsage();
        return -1;
    }
#if defined(CONFIG_PD2408_TEST_B_BOARD)
    else if (!strcmp(argv[1], "msg_example"))
    {
        ret = FFreeRTOSI2cMsgWriteThenReadCreate();
        if (ret != pdPASS)
        {
            printf("FFreeRTOSI2cMsgWriteThenReadCreate error :0x%x!\n",ret);
            return ret;
        }
    }
#endif
    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), i2c, I2cMsgCmdEntry, test freertos i2c msg driver);
#endif
