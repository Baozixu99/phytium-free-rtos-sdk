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
 * LastEditTime: 2023-07-07 15:40:40
 * Description:  This file is for providing the demo commond
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2022/12/20  Modify the format and establish the version
 *  1.1  Wangzq     2023/07/07  Add the multi-display config
 */
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"
#include "FreeRTOS.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"

#include "fdcdp.h"
#include "fdp_hw.h"

#include "lv_port_disp.h"
#include "lv_obj.h"
#include "lv_conf.h"
#include "lv_indev_create.h"
#include "lv_indev_test.h"

static void FFreeRTOSMediaCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("    Media init \r\n");
    printf("        -- init the dp \r\n");;
    printf("    Media lvgl-init \r\n");
    printf("        -- init the lvgl and set the para for demo\r\n");
    printf("    Media init-kb <id>\r\n");
    printf("        -- init the keyborad,test id is 0\r\n");
    printf("    Media init-ms <id>\r\n");
    printf("        -- init the mouse,test id is 1\r\n");
    printf("    Media demo \r\n");
    printf("        -- a test demo to use keyboard and mouse\r\n");
    printf("    Media lsusb \r\n");
    printf("        -- list the usb device\r\n");
}

static int MediaCmdEntry(int argc, char *argv[])
{
    u32 id ;
    u32 usb_id = 0;
    u32 channel;
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
    if (!strcmp(argv[1], "init-kb"))
    {
        if (argc < 3)
        {
            return -2;
        }
        if (inited != TRUE)
        {
            printf("please ensure the media has been inited \r\n");
            return -2;
        }
        usb_id = (uint8_t)simple_strtoul(argv[2], NULL, 10);
        BaseType_t task_ret = FFreeRTOSInitKbCreate(usb_id);

        if (pdPASS != task_ret)
        {
            return -2;
        }
    }
    if (!strcmp(argv[1], "init-ms"))
    {
        if (argc < 3)
        {
            return -2;
        }
        if (inited != TRUE)
        {
            printf("please ensure the media has been inited \r\n");
            return -2;
        }
        usb_id = (uint8_t)simple_strtoul(argv[2], NULL, 10);
        BaseType_t task_ret = FFreeRTOSInitMsCreate(usb_id);

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
        BaseType_t task_ret = FFreeRTOSDemoCreate();

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
        if (argc < 3)
        {
            return -2;
        }

        channel = (uint8_t)simple_strtoul(argv[2], NULL, 10);
        FFreeRTOSMediaChannelDeinit(channel);
    }

    if (!strcmp(argv[1], "lsusb"))
    {
        FFreeRTOSListUsbDev(argc - 1, &argv[1]);
    }
    return 0;

}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), Media, MediaCmdEntry, test freertos media driver);
#endif