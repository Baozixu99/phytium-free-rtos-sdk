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
 * FilePath: lwip_port.c
 * Date: 2022-04-06 14:46:52
 * LastEditTime: 2022-04-06 14:46:53
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#include <string.h>

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "lwipopts.h"

#ifdef CONFIG_LWIP_FXMAC
#include "ft_os_xmac.h"
#include "ethernetif.h"
#include "eth_ieee_reg.h"
#endif

#ifdef CONFIG_LWIP_FGMAC
#include "ethernetif.h"
#include "fgmac.h"
#include "fgmac_phy.h"
#include "fgmac_hw.h"
#endif

#include "lwip/mem.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/ip_addr.h"
#include "lwip/init.h"

#include "netif/etharp.h"
#include "lwip_port.h"
#include "parameters.h"
#include "f_printk.h"
#include "ft_debug.h"

#if !NO_SYS
#include "lwip/tcpip.h"
#endif

#define LWIP_PORT_DEBUG_TAG "LWIP-PORT"
#define LWIP_PORT_ERROR(format, ...)   FT_DEBUG_PRINT_E(LWIP_PORT_DEBUG_TAG, format, ##__VA_ARGS__)
#define LWIP_PORT_INFO(format, ...)    FT_DEBUG_PRINT_I(LWIP_PORT_DEBUG_TAG, format, ##__VA_ARGS__)
#define LWIP_PORT_DEBUG(format, ...)   FT_DEBUG_PRINT_D(LWIP_PORT_DEBUG_TAG, format, ##__VA_ARGS__)
#define LWIP_PORT_WARN(format, ...)    FT_DEBUG_PRINT_W(LWIP_PORT_DEBUG_TAG, format, ##__VA_ARGS__)


#if NO_SYS == 0
#define THREAD_STACKSIZE 256
#define LINK_DETECT_THREAD_INTERVAL 1000 /* one second */

void link_detect_thread(void *p);
#endif

/* Define those to better describe your network interface. */
#define IFNAME0 'f'
#define IFNAME1 't'

enum ethernet_link_status eth_link_status = ETH_LINK_UNDEFINED;
u32 phyaddrforemac;

volatile u32 timer_irq_cnt = 0;

/*
 * lwip_port_add: this is a wrapper around lwIP's netif_add function.
 * The objective is to provide portability between the different MAC's
 * This function can be used to add both xps_ethernetlite and xps_ll_temac
 * based interfaces
 */
struct netif *lwip_port_add(struct netif *netif,
	ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw,
	unsigned char *mac_ethernet_address,
	user_config *user_config)
{
	int i;
	
	if(user_config->magic_code != LWIP_PORT_CONFIG_MAGIC_CODE)
	{
		printf("user_config is illegitmacy \r\n");
		return NULL;
	}
#if NO_SYS == 0
	/* Start thread to detect link periodically for Hot Plug autodetect */
	sys_thread_new("link_detect_thread", link_detect_thread, netif,
			THREAD_STACKSIZE, tskIDLE_PRIORITY);
#endif

	/* set mac address */
	netif->hwaddr_len = 6;
	for (i = 0; i < 6; i++)
		netif->hwaddr[i] = mac_ethernet_address[i];

	netif->name[0] = IFNAME0;
  	netif->name[1] = IFNAME1;
	/* initialize based on MAC type */
	return netif_add(netif, 
#if LWIP_IPV4
		ipaddr, netmask, gw,
#endif
		(void*)(uintptr)user_config,
		ethernetif_init,
#if NO_SYS
		ethernet_input
#else
		tcpip_input
#endif
		);

}

#if !NO_SYS
/*
 * The input thread calls lwIP to process any received packets.
 * This thread waits until a packet is received (sem_rx_data_available),
 * and then calls lwip_port_input which processes 1 packet at a time.
 */
void lwip_port_input_thread(struct netif *netif)
{
	struct LwipPort *emac = (struct LwipPort *)netif->state;
	
	while (1) {
		/* sleep until there are packets to process
		 * This semaphore is set by the packet receive interrupt
		 * routine.
		 */
		sys_arch_sem_wait(&emac->sem_rx_data_available,0);
		/* move all received packets to lwIP */
		lwip_port_input(netif);
	}
}
#endif

void lwip_port_input(struct netif *netif)
{
	ethernetif_input(netif);
}

#if defined(CONFIG_LWIP_FXMAC)

extern enum ethernet_link_status FXmacPhyReconnect(struct LwipPort *xmac_netif_p);

void lwip_port_link_detect(struct netif *netif)
{
	enum ethernet_link_status status;
	struct LwipPort *xmac_netif_p = (struct LwipPort *)(netif->state);
	FXmacOs *instance_p;
	if(xmac_netif_p == NULL)
	{
		return;
	}
	instance_p = (FXmacOs *)xmac_netif_p->state;
	if (instance_p->instance.is_ready != FT_COMPONENT_IS_READY)
	{
		return;
	}

	status = FXmacPhyReconnect(xmac_netif_p);

	switch(status)
	{
		case ETH_LINK_UP:
			if(netif_is_link_up(netif) == 0)
			{
				LWIP_PORT_INFO("link up"); 
				netif_set_link_up(netif) ;
			}
		break;
		case ETH_LINK_DOWN:
		default:
			if(netif_is_link_up(netif) == 1)
			{
				LWIP_PORT_INFO("link down"); 
				netif_set_link_down(netif) ;
			}
			break;		
	}
}


#if NO_SYS == 0

void link_detect_thread(void *p)
{
	struct netif *netif = (struct netif *) p;
	
	while (1) {
		/* Call eth_link_detect() every second to detect Ethernet link
		 * change.
		 */
		lwip_port_link_detect(netif);
		vTaskDelay(LINK_DETECT_THREAD_INTERVAL / portTICK_RATE_MS);
	}
}


#endif


void lwip_port_start(struct netif *netif)
{
	u32 link_speed, phy_link_status;
	enum ethernet_link_status link_status;

	struct ethernetif *mac_netif = (struct ethernetif *)(netif->state);
	
}

void lwip_port_stop(struct netif *netif)
{
	u32 link_speed, phy_link_status;
	enum ethernet_link_status link_status;

	struct ethernetif *mac_netif = (struct ethernetif *)(netif->state);
	
}


#else

static u32 phy_link_detect(FGmac *instance_p, u32 phy_addr)
{
	u16 status;

	/* Read Phy Status register twice to get the confirmation of the current
	 * link status.
	 */
	FGmacReadPhyReg(instance_p, instance_p->phy_addr, FGMAC_PHY_MII_STATUS_REG, &status);
	FGmacReadPhyReg(instance_p, instance_p->phy_addr, FGMAC_PHY_MII_STATUS_REG, &status);

	if (status & FGMAC_PHY_MII_SR_LSTATUS)
		return 1;
	return 0;
}

static u32 phy_autoneg_status(FGmac *instance_p, u32 phy_addr)
{
	u16 status;

	/* Read Phy Status register twice to get the confirmation of the current
	 * link status.
	 */
	FGmacReadPhyReg(instance_p, instance_p->phy_addr, FGMAC_PHY_MII_STATUS_REG, &status);
	FGmacReadPhyReg(instance_p, instance_p->phy_addr, FGMAC_PHY_MII_STATUS_REG, &status);

	if (status & FGMAC_PHY_MII_SR_AUTO_NEGOT_COMPLETE)
		return 1;
	return 0;
}

void lwip_port_link_detect(struct netif *netif)
{
	u32 link_speed, phy_link_status;
	enum ethernet_link_status link_status;

	struct ethernetif *mac_netif = (struct ethernetif *)(netif->state);
	FGmac *instance_p = mac_netif->ethctrl;

	if (instance_p->is_ready != FT_COMPONENT_IS_READY)
	{
		LWIP_PORT_ERROR("instance_p is not ready\n");
		return;
	}

	/* read gmac phy link status */
	phy_link_status = phy_link_detect(instance_p, instance_p->phy_addr);
	link_status = phy_link_status ? ETH_LINK_UP : ETH_LINK_DOWN;

	/* if the link status is changed */
	if(eth_link_status != link_status)
		eth_link_status = link_status;
	else
		return;

	switch (eth_link_status) 
	{
		case ETH_LINK_UP:
			if (phy_link_status && phy_autoneg_status(instance_p, instance_p->phy_addr)) 
			{
				netif_set_link_up(netif);
				eth_link_status = ETH_LINK_UP;
				LWIP_PORT_DEBUG("Ethernet Link up\r\n");
			}
			break;
		case ETH_LINK_DOWN:
			netif_set_link_down(netif);
			LWIP_PORT_DEBUG("Ethernet Link down\r\n");
			break;
		default:
			break;
	}

	return;
}


void lwip_port_start(struct netif *netif)
{
	u32 link_speed, phy_link_status;
	enum ethernet_link_status link_status;

	struct ethernetif *mac_netif = (struct ethernetif *)(netif->state);
	FGmac *instance_p = mac_netif->ethctrl;

	if (instance_p->is_ready != FT_COMPONENT_IS_READY)
	{
		LWIP_PORT_ERROR("instance_p is not ready\n");
		return;
	}

	FGmacStartTrans(instance_p);
}

void lwip_port_stop(struct netif *netif)
{
	u32 link_speed, phy_link_status;
	enum ethernet_link_status link_status;

	struct ethernetif *mac_netif = (struct ethernetif *)(netif->state);
	FGmac *instance_p = mac_netif->ethctrl;

	if (instance_p->is_ready != FT_COMPONENT_IS_READY)
	{
		LWIP_PORT_ERROR("instance_p is not ready\n");
		return;
	}

	FGmacStopTrans(instance_p);
}

#endif


extern volatile unsigned int gCpuRuntime;

u32_t sys_now(void)
{
	return gCpuRuntime;
}
