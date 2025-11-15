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
 * FilePath: if_set.c
 * Date: 2022-10-27 16:41:01
 * LastEditTime: 2022-10-27 16:41:02
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */
#include "sdkconfig.h"
#include "ftypes.h"
#include "stdio.h"
#include "string.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
#include "lwip/inet.h"
#include "lwip/init.h"
#include "sys_arch.h"
int Max_mtu = -1;
void SetIf(char* netif_name, char* ip_addr, char* gw_addr, char* nm_addr)
{
    sys_prot_t cur;
    cur = sys_arch_protect();

#if LWIP_VERSION_MAJOR == 1U /* v1.x */
    struct ip_addr *ip;
    struct ip_addr addr;
#else /* >= v2.x */
    ip4_addr_t *ip;
    ip4_addr_t addr;
#endif /* LWIP_VERSION_MAJOR == 1U */
    struct netif *netif = netif_list;

    if(strlen(netif_name) > sizeof(netif->name))
    {
        printf("network interface name too long! %d %s \r\n",strlen(netif_name),netif_name);
        goto exit;
    }

    while(netif != NULL)
    {
        if(strncmp(netif_name, netif->name, sizeof(netif->name)) == 0)
            break;

        netif = netif->next;
        if( netif == NULL )
        {
            printf("network interface: %s not found!\r\n", netif_name);
            goto exit;
        }
    }
#if LWIP_VERSION_MAJOR == 1U /* v1.x */
    ip = (struct ip_addr *)&addr;
#else /* >= v2.x */
    ip = (ip4_addr_t *)&addr;
#endif /* LWIP_VERSION_MAJOR == 1U */

    /* set ip address */
    if ((ip_addr != NULL) && inet_aton(ip_addr, &addr))
    {
        netif_set_ipaddr(netif, ip);
    }

    /* set gateway address */
    if ((gw_addr != NULL) && inet_aton(gw_addr, &addr))
    {
        netif_set_gw(netif, ip);
    }

    /* set netmask address */
    if ((nm_addr != NULL) && inet_aton(nm_addr, &addr))
    {
        netif_set_netmask(netif, ip);
    }
exit:
    sys_arch_unprotect(cur);
}


void SetMtu(char * netif_name , char * Mtu_value)
{
    sys_prot_t cur;
    cur = sys_arch_protect();
    struct netif *netif = netif_list;

    if(strlen(netif_name) > sizeof(netif->name))
    {
        printf("network interface name too long! %d %s \r\n",strlen(netif_name),netif_name);
        goto exit;
    }
    while(netif != NULL)
    {
        if(strncmp(netif_name, netif->name, sizeof(netif->name)) == 0)
            break;

        netif = netif->next;
        if( netif == NULL )
        {
            printf("network interface: %s not found!\r\n", netif_name);
            goto exit;
        }
    }
    if(!Mtu_value)
    {
        printf("Input error : Missing max_value parameters! \n");
        goto exit;
    }
    if(Max_mtu == -1)
    {
        Max_mtu = netif->mtu;
    }
    
    int temp = atoi(Mtu_value);
    if (temp <= 0)
    {
        printf("Error: The mtu value input is wrong!\n");
    }
    else if(temp > Max_mtu)
    {
        printf("Error: The mtu value can not exceed %d !\n",Max_mtu);
    }
    else
    {
        netif->mtu = temp;
        printf("Mtu changed ,now is %d \n", netif->mtu);
    }

exit:
    sys_arch_unprotect(cur);
}

