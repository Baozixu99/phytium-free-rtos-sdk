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
 * FilePath: if_list.c
 * Date: 2022-10-27 16:20:55
 * LastEditTime: 2022-10-27 16:20:55
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */
#include "sdkconfig.h"
#include "ftypes.h"
#include "stdio.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
#include "lwip/inet.h"
#include "lwip/init.h"
#include "sys_arch.h"


void ListIf(void)
{
    u8 index;
    struct netif * netif;
    sys_prot_t cur;
    cur = sys_arch_protect();

    netif = netif_list;

    while( netif != NULL )
    {
        printf("network interface: %c%c%s\n",
                   netif->name[0],
                   netif->name[1],
                   (netif == netif_default)?" (Default)":"");
        printf("MTU: %d\n", netif->mtu);
        printf("MAC: ");
        for (index = 0; index < netif->hwaddr_len; index ++)
            printf("%02x ", netif->hwaddr[index]);
        printf("\nFLAGS:");
        if (netif->flags & NETIF_FLAG_UP) printf(" UP");
        else printf(" DOWN");
        if (netif->flags & NETIF_FLAG_LINK_UP) printf(" LINK_UP");
        else printf(" LINK_DOWN");
        if (netif->flags & NETIF_FLAG_ETHARP) printf(" ETHARP");
        if (netif->flags & NETIF_FLAG_BROADCAST) printf(" BROADCAST");
        if (netif->flags & NETIF_FLAG_IGMP) printf(" IGMP");
        printf("\n");
        printf("ip address: %s\n", ipaddr_ntoa(&(netif->ip_addr)));
        printf("gw address: %s\n", ipaddr_ntoa(&(netif->gw)));
        printf("net mask  : %s\n", ipaddr_ntoa(&(netif->netmask)));
        printf("\n");
#if LWIP_IPV6
        {
            ip6_addr_t *addr;
            int addr_state;
            int i;

            addr = (ip6_addr_t *)&netif->ip6_addr[0];
            addr_state = netif->ip6_addr_state[0];

            printf("\nipv6 link-local: %s state:%02X %s\n", ip6addr_ntoa(addr),
            addr_state, ip6_addr_isvalid(addr_state)?"VALID":"INVALID");

            for(i=1; i<LWIP_IPV6_NUM_ADDRESSES; i++)
            {
                addr = (ip6_addr_t *)&netif->ip6_addr[i];
                addr_state = netif->ip6_addr_state[i];

                printf("ipv6[%d] address: %s state:%02X %s\n", i, ip6addr_ntoa(addr),
                addr_state, ip6_addr_isvalid(addr_state)?"VALID":"INVALID");
            }
        }
        printf("\r\n");
#endif /* LWIP_IPV6 */
        netif = netif->next;
    }

#if LWIP_DNS
    {
#if LWIP_VERSION_MAJOR == 1U /* v1.x */
        struct ip_addr ip_addr;

        for(index=0; index<DNS_MAX_SERVERS; index++)
        {
            ip_addr = dns_getserver(index);
            printf("dns server #%d: %s\n", index, ipaddr_ntoa(&(ip_addr)));
        }
#else /* >= v2.x */
        const ip_addr_t *ip_addr;

        for(index=0; index<DNS_MAX_SERVERS; index++)
        {
            ip_addr = dns_getserver(index);
            printf("dns server #%d: %s\n", index, inet_ntoa(ip_addr));
        }
#endif /* LWIP_VERSION_MAJOR == 1U */
    }
#endif /**< #if LWIP_DNS */

    sys_arch_unprotect(cur);

}

