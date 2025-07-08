/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: cmd_qspi_spiffs.c
 * Date: 2024-04-25 14:06:43
 * LastEditTime: 2024-04-25 14:06:43
 * Description:  This files is for qspi spiffs command interface
 *
 * Modify History:
 *  Ver      Who           Date         Changes
 * -----    ------       --------      --------------------------------------
 *  1.0     huangjin     2024/04/25    add no letter shell mode, adapt to auto-test system
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "strto.h"

#include "sdkconfig.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"

#include "qspi_spiffs_example.h"
#endif
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
#ifdef CONFIG_USE_LETTER_SHELL
static void QspiSpiffsCmdUsage()
{
    printf("Usage:\r\n");
    printf("qspi_spiffs write_read\r\n");
    printf("-- Demo write and read by qspi_spiffs\r\n");
}

static int QspiSpiffsCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        QspiSpiffsCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "write_read"))
    {
        ret = FFreeRTOSQspiSpiffsCreate();
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), qspi_spiffs, QspiSpiffsCmdEntry, test freertos qspi_spiffs);
#endif