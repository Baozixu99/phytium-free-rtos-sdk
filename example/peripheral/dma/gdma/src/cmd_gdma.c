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
 * FilePath: cmd_gdma.c
 * Date: 2022-07-14 14:06:43
 * LastEditTime: 2022-07-14 14:06:43
 * Description:  This files is for gdma command interface
 *
 * Modify History:
 *  Ver      Who           Date         Changes
 * -----    ------       --------      --------------------------------------
 *  1.0    zhugengyu     2022/7/27     init commit
 *  2.0    liqiaozhong   2024/4/22     add no letter shell mode, adapt to auto-test system
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "strto.h"

#include "sdkconfig.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"

#include "gdma_memcpy.h"
#endif
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
#ifdef CONFIG_USE_LETTER_SHELL
static void GdmaCmdUsage()
{
    printf("Usage:\r\n");
    printf("gdma memcpy\r\n");
    printf("-- Run GDMA memcpy example\r\n");
}

static int GdmaCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        GdmaCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "memcpy"))
    {
        ret = FFreeRTOSGdmaMemcpy();
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), gdma, GdmaCmdEntry, test freertos gdma driver);
#endif