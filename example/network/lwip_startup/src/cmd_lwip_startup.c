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
 * FilePath: cmd_lwip_startup.c
 * Created Date: 2023-11-21 11:06:40
 * Last Modified: 2024-04-26 15:52:43
 * Description:  This file is for lwip startup example cmd catalogue.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong   2023/12/26          first release
 *  2.0   liuzhihong   2024/4/26   add no letter shell mode, adapt to auto-test system
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

#include "lwip_dhcp_example.h"
#include "lwip_ipv4_example.h"
#include "lwip_ipv6_example.h"


#define EXAMPLE_IDLE 0
#define IPV4_EXAMPLE_RUNNING 1
#define IPV6_EXAMPLE_RUNNING 2
#define DHCP_EXAMPLE_RUNNING 3

static u32 init_flag_mask=EXAMPLE_IDLE;


static void  LwipStartupExampleCheckState(void)
{
    switch(init_flag_mask)
    {
        case IPV4_EXAMPLE_RUNNING:
            printf("Lwip ipv4 example is running, we need to deinitialize it first! \r\n");
            LwipIpv4TestDeinit();
            init_flag_mask=EXAMPLE_IDLE;
            break;
        case IPV6_EXAMPLE_RUNNING:
            printf("Lwip ipv6 example is running, we need to deinitialize it first! \r\n");
            LwipIpv6TestDeinit();
            init_flag_mask=EXAMPLE_IDLE;
            break;
        case DHCP_EXAMPLE_RUNNING:
            printf("Lwip dhcp example is running, we need to deinitialize it first! \r\n"); 
            LwipDhcpTestDeinit();
            init_flag_mask=EXAMPLE_IDLE;
            break;
        default:
            break;
    }
}

/* usage info function for lwip startup example */
static void LwipStartupExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("lwip ipv4\r\n");
    printf("-- run lwip ipv4 mode example to initialize mac controller\r\n");
    printf("lwip ipv6\r\n");
    printf("-- run lwip ipv6 mode example to initialize mac controller\r\n");
    printf("lwip dhcp\r\n");
    printf("-- run lwip dhcp mode example to initialize mac controller\r\n");
}

/* entry function for lwip startup example */
static int LwipStartupExampleEntry(int argc, char *argv[])
{
    int ret = 0;

    /* check input args of example, exit if invaild */
    if (argc < 2)
    {
        LwipStartupExampleUsage();
        return -1;
    }

    /* parser example input args and run example */
    if (!strcmp(argv[1], "ipv4"))
    {
        LwipStartupExampleCheckState();
        ret = FFreeRTOSLwipIpv4TaskCreate();
        init_flag_mask = IPV4_EXAMPLE_RUNNING;       
    }
    else if (!strcmp(argv[1], "ipv6"))
    {
        LwipStartupExampleCheckState();
        ret = FFreeRTOSLwipIpv6TaskCreate();
        init_flag_mask = IPV6_EXAMPLE_RUNNING;
    }
    else if (!strcmp(argv[1], "dhcp"))
    {
        LwipStartupExampleCheckState();
        ret = FFreeRTOSLwipDhcpTaskCreate();
        init_flag_mask = DHCP_EXAMPLE_RUNNING;
    }

    return ret;
}

/* register command for lwip startup example */
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), lwip, LwipStartupExampleEntry, lwip startup example);
#endif