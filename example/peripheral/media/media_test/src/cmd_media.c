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
 * FilePath: cmd_meida.c
 * Date: 2023-02-15 14:34:44
 * LastEditTime: 2023-02-16 14:34:45
 * Description:  This file is for media shell command.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangzq  2023/02/16  init commit
 */
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"

#include "FreeRTOS.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"

#include "media_example.h"


static void FFreeRTOSMediaCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("    Media init \r\n");
    printf("        -- init the dp \r\n");;
    printf("    Media deinit \r\n");
    printf("        -- deinit the dp \r\n");;  
    printf("    Media demo\r\n");
    printf("        -- a demo to light the screen \r\n");;

}
static int MediaCmdEntry(int argc, char *argv[])
{
    u32 id ;
    static boolean inited = FALSE;
    if (argc < 2)
    {
        FFreeRTOSMediaCmdUsage();
        return -1;
    }
    if (!strcmp(argv[1], "init"))
    {
 
        BaseType_t task_ret = FFreeRTOSMediaCreate();

        if (pdPASS != task_ret)
        {
            return -2;
        }
        inited = TRUE;
    }
    if (!strcmp(argv[1], "demo"))
    {
        if (inited != TRUE)
        {
            printf("please ensure the media has been inited \r\n");
            return -2;
        }
        FMediaDisplayDemo();
    }
    if (!strcmp(argv[1], "deinit"))
    {
        if (inited != TRUE)
        {
            printf("please ensure the media has been inited \r\n");
            return -2;
        }
        if (argc >= 3)
        {
            id = (u32)simple_strtoul(argv[2], NULL, 10);
        }
        FFreeRTOSMediaChannelDeinit(id);
    }

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), Media, MediaCmdEntry, test freertos media driver);
#endif