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
 * FilePath: cmd_lwip_iperf.c
 * Created Date: 2023-10-16 15:16:18
 * Last Modified: 2024-04-28 10:31:08
 * Description:  This file is for lwip iperf example cmd catalogue.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong  2023/10/16          first release
 *  2.0   liuzhihong  2024/04/28   add no letter shell mode, adapt to auto-test system
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

#include "lwip_iperf_client_example.h"
#include "lwip_iperf_server_example.h"




#define EXAMPLE_IDLE 0
#define CLIENT_EXAMPLE_RUNNING 1
#define SERVER_EXAMPLE_RUNNING 2


static u32 init_flag_mask = EXAMPLE_IDLE;


static void  LwipIperfExampleCheckState(void)
{
    switch(init_flag_mask)
    {
        case CLIENT_EXAMPLE_RUNNING:
            printf("Lwip iperf client example is running, we need to deinitialize it first! \r\n");
            LwipIperfClientDeinit();
            init_flag_mask=EXAMPLE_IDLE;
            break;
        case SERVER_EXAMPLE_RUNNING:
            printf("Lwip iperf server example is running, we need to deinitialize it first! \r\n");
            LwipIperfServerDeinit();
            init_flag_mask=EXAMPLE_IDLE;
            break;
        default:
            break;
    }
}

/* usage info function for lwip iperf example */
static void LwipIperfExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("lwip iperfc \r\n");
    printf("-- run lwip ipv4 mode example to initialize mac controller and run iperf client test\r\n");
    printf("lwip iperfs \r\n");
    printf("-- run lwip ipv4 mode example to initialize mac controller and run iperf server test\r\n");
}

/* entry function for lwip iperf example */
static int LwipIperfExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        LwipIperfExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "iperfc"))
    {
        LwipIperfExampleCheckState();
        ret = FFreeRTOSLwipIperfClientTaskCreate();
        init_flag_mask = CLIENT_EXAMPLE_RUNNING;       
    }
    else if (!strcmp(argv[1], "iperfs"))
    {
        LwipIperfExampleCheckState();
        ret = FFreeRTOSLwipIperfServerTaskCreate();
        init_flag_mask = SERVER_EXAMPLE_RUNNING;       
    }

    return ret;
}

/* register command for lwip iperf example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), lwip, LwipIperfExampleEntry, lwip iperf example);
#endif