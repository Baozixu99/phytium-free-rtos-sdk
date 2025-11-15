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
 * FilePath: if_cmd.c
 * Date: 2022-10-27 16:41:15
 * LastEditTime: 2022-10-27 16:41:15
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */
#include "ftypes.h"
#include <stdio.h>
#include <string.h>
#include "ifconfig.h"
#include "sdkconfig.h"


#if defined(CONFIG_USE_LETTER_SHELL)
#include "../src/shell.h"

static void IfCmdUsage(void)
{
    printf("usage:\r\n");
    printf(" -- if you only input ifconfig ,list the information of all network interfaces \r\n");
    printf(" -- if you input ifconfig  <netif name> mtu <mtu value>, you can change the mtu of corresponding netif \r\n");
    printf("        -- <mtu value> mtu value of netif. Attention: This value can not exceed the preset value when we initialize \r\n");
    printf(" -- if you input ifconfig <netif name> <Ip address> <Gateway> <Netmask> ,you can change the ip of corresponding netif \r\n");
    printf("        -- <netif name> lwip netif name \r\n");
    printf("        -- <Ip address> ip address of netif \r\n");
    printf("        -- <Gateway> Gateway of netif \r\n");
    printf("        -- <Netmask> Netmask of netif \r\n");
    printf("\r\n\r\n");
}

static int  SetIfCmd(int argc, char *argv[])
{

    char *netif_name = NULL;
    char * ip_addr = NULL;
    char * gw_addr = NULL;
    char * nm_addr = NULL;
    char * mtu_value = NULL;
    char * mtu_flag = NULL;
    printf("argc is %d\r\n", argc);
    switch (argc)
    {
        case 5:
            nm_addr = argv[4];
        case 4:
            gw_addr = argv[3];
            mtu_value = argv[3];
        case 3:
            ip_addr = argv[2];
            mtu_flag = argv[2];
        case 2:
            netif_name = argv[1];
            break;
        default:
            break;
    }

    if(strcmp(mtu_flag ,"mtu") == 0)
    {
        SetMtu(netif_name, mtu_value);
    }
    else
    {
        SetIf(netif_name, ip_addr, gw_addr, nm_addr);
    } 
    return 0;
}

static void if_cmd(int argc, char *argv[])
{
    if (argc <= 1)
    {
        IfCmdUsage();
        ListIf();
    }
    else
    {
         SetIfCmd(argc, argv);  
    }
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), ifconfig, if_cmd, list network interface information);

#endif
