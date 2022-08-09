/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * Date: 2022-07-21 11:05:57
 * LastEditTime: 2022-07-21 11:05:57
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */


#ifndef __LWIP_PORT_H_
#define __LWIP_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "ft_types.h"
#include "lwipopts.h"

#include "lwip/netif.h"
#include "lwip/ip.h"


/* Mii interface */
#define LWIP_PORT_INTERFACE_RGMII 0
#define LWIP_PORT_INTERFACE_SGMII 1

/* Phy speed */
#define LWIP_PORT_SPEED_10M    10
#define LWIP_PORT_SPEED_100M    100
#define LWIP_PORT_SPEED_1000M    1000

/* Duplex */
#define LWIP_PORT_HALF_DUPLEX    0
#define LWIP_PORT_FULL_DUPLEX    1

#define LWIP_PORT_CONFIG_MAGIC_CODE 0x616b6200

typedef struct
{
	u32 magic_code;		/* LWIP_PORT_CONFIG_MAGIC_CODE */
	u32 mac_instance;
	u32 mii_interface;  /* LWIP_PORT_INTERFACE_XXX */
    u32 autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    u32 phy_speed;  /* LWIP_PORT_SPEED_XXX */
    u32 phy_duplex; /* LWIP_PORT_XXX_DUPLEX */
} user_config;

#define LWIP_PORT_CONFIG_DEFAULT_INIT(config)           \
	do                                                    \
	{                                                     \
		config.magic_code = LWIP_PORT_CONFIG_MAGIC_CODE;  \
		config.mac_instance = 3;                          \
		config.mii_interface = LWIP_PORT_INTERFACE_RGMII; \
		config.autonegotiation = 1;                       \
		config.phy_speed = LWIP_PORT_SPEED_1000M;         \
		config.phy_duplex = LWIP_PORT_FULL_DUPLEX; \
}while(0)

struct LwipPort {
	void *state;
	
#if !NO_SYS
    sys_sem_t sem_rx_data_available;
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
	user_config *user_config);

void lwip_port_start(struct netif *netif);
void lwip_port_stop(struct netif *netif);

#ifdef __cplusplus
}
#endif

#endif
