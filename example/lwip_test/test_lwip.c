/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
 * All Rights Reserved.
 *  
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it  
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
 * either version 1.0 of the License, or (at your option) any later version. 
 *  
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details. 
 *  
 * 
 * FilePath: test_lwip.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:02:05
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ft_types.h"

#ifndef SDK_CONFIG_H__
	#error "Please include sdkconfig.h first"
#endif

#ifndef CONFIG_ENABLE_FGMAC
	#error "Please include component f_gmac first"
#endif

#include "fgmac.h"
#include "lwip/ip4_addr.h"
#include "lwip/init.h"
#include "netif/ethernet.h"
#include "ethernetif.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "ft_os_gmac.h"

/* board ip addrress */
#define IP_ADDR0        192
#define IP_ADDR1        168 
#define IP_ADDR2        4
#define IP_ADDR3        20 

/* netmask */
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/* gateway */
#define GATEWAY_ADDR0   192
#define GATEWAY_ADDR1   168
#define GATEWAY_ADDR2   4
#define GATEWAY_ADDR3   1

struct ethernetif {
    struct eth_addr *ethaddr;
    FGmac *ethctrl;
};

static ip4_addr_t ip_addr, net_mask, gate_way;

void LwipRawTest(FtOsGmac *os_gmac)
{
    memset(&ip_addr, 0, sizeof(ip_addr));
    memset(&net_mask, 0, sizeof(net_mask));
    memset(&gate_way, 0, sizeof(gate_way));

    IP4_ADDR(&ip_addr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    IP4_ADDR(&net_mask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
    IP4_ADDR(&gate_way, GATEWAY_ADDR0, GATEWAY_ADDR1, GATEWAY_ADDR2, GATEWAY_ADDR3);
   
    /* 初始化LwIP堆 */
    tcpip_init(NULL, NULL);
    
    /* 添加网络接口 (IPv4/IPv6)  */
    netif_add(&os_gmac->netif_object, &ip_addr, &net_mask, &gate_way, NULL, ethernetif_init, tcpip_input);
    
    /* 注册默认网络接口 */
    netif_set_default(&os_gmac->netif_object);
    
    if (netif_is_link_up(&os_gmac->netif_object))
    {
        /* 当netif完全配置好时，必须调用该函数 */
        printf("link is up...\r\n");
        netif_set_up(&os_gmac->netif_object);
    }
    else
    {
        /* 当netif链接关闭时，必须调用该函数 */
        printf("link is down..\r\n");
        netif_set_down(&os_gmac->netif_object);
    }

}
