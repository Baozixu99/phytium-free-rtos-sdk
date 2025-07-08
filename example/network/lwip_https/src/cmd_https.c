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
 * FilePath: cmd_https.c
 * Created Date: 2024-9-29 11:06:40
 * Last Modified: 2025-05-20 10:27:20
 * Description:  This file is for https example cmd catalogue.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   huangjin     2024/9/29          first release
 */

#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "strto.h"

#include "https_example.h"

#define EXAMPLE_IDLE 0
#define HTTPS_EXAMPLE_RUNNING 1

static u32 init_flag_mask = EXAMPLE_IDLE;

static void  HttpsExampleCheckState(void)
{
    switch(init_flag_mask)
    {
        case HTTPS_EXAMPLE_RUNNING:
            printf("Https example is running, we need to deinitialize it first! \r\n");
            HttpsTestDeinit();
            init_flag_mask = EXAMPLE_IDLE;
            break;
        default:
            break;
    }
}

/* usage info function for https example */
static void HttpsExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("lwip https\r\n");
    printf("-- run https example to initialize mac controller\r\n");
}

/* entry function for https example */
static int HttpsExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        HttpsExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "https"))
    {
        HttpsExampleCheckState();
        ret = FFreeRTOSHttpsTaskCreate();
        init_flag_mask = HTTPS_EXAMPLE_RUNNING;       
    }

    return ret;
}

/* register command for https example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), lwip, HttpsExampleEntry, https example);
#endif