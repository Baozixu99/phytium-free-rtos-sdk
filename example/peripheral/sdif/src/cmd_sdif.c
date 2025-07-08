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
 * FilePath: cmd_sdif.c
 * Date: 2022-07-12 09:33:12
 * LastEditTime: 2022-07-12 09:33:12
 * Description:  This file is for providing user command functions.
 *
 * Modify History:
 *  Ver      Who           Date         Changes
 * -----    ------       --------      --------------------------------------
 *  1.0    zhugengyu     2022/8/26     first commit
 *  2.0    liqiaozhong   2024/4/22     add no letter shell mode, adapt to auto-test system
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "strto.h"

#include "sdkconfig.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"
#include "sdif_example.h"
#endif
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
#ifdef CONFIG_USE_LETTER_SHELL
static void SdCmdUsage()
{
    printf("Usage:\r\n");
    printf("sd tf\r\n");
    printf("-- Demo read and write by sdmmc\r\n");
    printf("sd emmc\r\n");
    printf("-- Demo read and write by sdmmc\r\n");
}

static int SdCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        SdCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "tf"))
    {
        ret = FFreeRTOSTfWriteRead();
    }
    else if (!strcmp(argv[1], "emmc"))
    {
        ret = FFreeRTOSEmmcWriteRead();
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), sd, SdCmdEntry, test freertos sd rw);
#endif