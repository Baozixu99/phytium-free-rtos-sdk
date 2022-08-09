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
 * FilePath: lwip_port.h
 * Date: 2022-04-06 14:46:52
 * LastEditTime: 2022-04-06 14:46:53
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef __LWIP_PORT_H_
#define __LWIP_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lwipopts.h"

#if !NO_SYS
#ifdef OS_IS_XILKERNEL
#include "xmk.h"
#endif
#include "lwip/sys.h"
#endif

#include "lwip/netif.h"
#include "lwip/ip.h"

struct xmac_netif {
	void *state;
#if !NO_SYS
    sys_sem_t sem_rx_data_available;
#endif
#if defined(OS_IS_FREERTOS) && defined(__arm__) && !defined(ARMR5)
	TimerHandle_t xTimer;
#endif
};

enum ethernet_link_status {
	ETH_LINK_UNDEFINED = 0,
	ETH_LINK_UP,
	ETH_LINK_DOWN,
	ETH_LINK_NEGOTIATING
};

void lwip_port_link_detect(struct netif *netif);
void lwip_port_input(struct netif *netif);
void lwip_port_input_thread(struct netif *netif);
struct netif *lwip_port_add(struct netif *netif,
	ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw,
	unsigned char *mac_ethernet_address,
	unsigned int mac_id);

void lwip_port_start(struct netif *netif);
void lwip_port_stop(struct netif *netif);

#if defined(__arm__) || defined(__aarch64__)
void lwip_port_resetrx_on_no_rxdata(struct netif *netif);
#endif

#ifdef __cplusplus
}
#endif

#endif
