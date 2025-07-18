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
 * FilePath: cmd.c
 * Date: 2022-08-25 16:22:40
 * LastEditTime: 2023-03-20 15:40:40
 * Description:  This file is for providing the demo commond
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2022/12/20  Modify the format and establish the version
 *  1.1  Wangzq     2023/03/20  Add the multi-display config
 */
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"
#include "FreeRTOS.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"

#include "lv_demo_create.h"
static void FFreeRTOSMediaCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("    Media init \r\n");
    printf("        -- init the Media\r\n");
    printf("    Media lvgl-init \r\n");
    printf("        -- init the lvgl and set the para for demo\r\n");
    printf("    Media demo \r\n");
    printf("        -- a test demo for user to comprehend the driver\r\n");
    printf("    Media deinit \r\n");
    printf("        -- deinit the channel\r\n");
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
        BaseType_t task_ret = FFreeRTOSMediaInitCreate();

        if (pdPASS != task_ret)
        {
            return -2;
        }
        inited = TRUE;

    }
    if (!strcmp(argv[1], "lvgl-init"))
    {
        if (inited != TRUE)
        {
            printf("please ensure the media has been inited \r\n");
            return -2;
        }
        BaseType_t task_ret = FFreeRTOSlVGLConfigCreate();

        if (pdPASS != task_ret)
        {
            return -2;
        }
    }
    if (!strcmp(argv[1], "demo"))
    {
        if (inited != TRUE)
        {
            printf("please ensure the media has been inited \r\n");
            return -2;
        }
        BaseType_t task_ret = FFreeRTOSlVGLDemoCreate();

        if (pdPASS != task_ret)
        {
            return -2;
        }
    }
    if (!strcmp(argv[1], "deinit"))
    {
        if (inited != TRUE)
        {
            printf("please ensure the media has been inited \r\n");
            return -2;
        }
        BaseType_t task_ret =  FFreeRTOSMediaDeinit();
        if (pdPASS != task_ret)
        {
            return -2;
        }
    }

    return 0;
}

SHELL_EXPORT_CMD((SHELL_TYPE_CMD_MAIN), Media, MediaCmdEntry, test media driver);
#endif