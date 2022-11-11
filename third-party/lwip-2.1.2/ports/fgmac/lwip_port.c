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
 * FilePath: lwip_port.c
 * Date: 2022-06-09 16:03:57
 * LastEditTime: 2022-06-09 16:03:57
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver Who      Date        Changes
 * ----- ------     --------    --------------------------------------
 */

#include <string.h>

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "lwipopts.h"

#ifdef CONFIG_LWIP_FXMAC
#include "fxmac.h"
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
#include "fparameters.h"
#include "ftypes.h"
#include "fdebug.h"
#include "fgeneric_timer.h"

#if !NO_SYS
#include "lwip/tcpip.h"
#endif

#ifdef OS_IS_FREERTOS
#define THREAD_STACKSIZE 256
#define LINK_DETECT_THREAD_INTERVAL 1000 /* one second */

void link_detect_thread(void *p);
#endif

#define LWIP_PORT_DEBUG_TAG "LWIP-PORT"
#define LWIP_PORT_ERROR(format, ...)   FT_DEBUG_PRINT_E(LWIP_PORT_DEBUG_TAG, format, ##__VA_ARGS__)
#define LWIP_PORT_INFO(format, ...)    FT_DEBUG_PRINT_I(LWIP_PORT_DEBUG_TAG, format, ##__VA_ARGS__)
#define LWIP_PORT_DEBUG(format, ...)   FT_DEBUG_PRINT_D(LWIP_PORT_DEBUG_TAG, format, ##__VA_ARGS__)
#define LWIP_PORT_WARN(format, ...)    FT_DEBUG_PRINT_W(LWIP_PORT_DEBUG_TAG, format, ##__VA_ARGS__)

/* Define those to better describe your network interface. */
#define IFNAME0 'f'
#define IFNAME1 't'

enum ethernet_link_status eth_link_status = ETH_LINK_UNDEFINED;
u32 phyaddrforemac;

volatile u32 timer_irq_cnt = 0;

/*
 * lwip_port_add: this is a wrapper around lwIP's netif_add function.
 * The objective is to provide portability between the different Xilinx MAC's
 * This function can be used to add both xps_ethernetlite and xps_ll_temac
 * based interfaces
 */
struct netif *lwip_port_add(struct netif *netif,
	ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw,
	unsigned char *mac_ethernet_address,
	unsigned int mac_id)
{
	u8 i;

#ifdef OS_IS_FREERTOS
	/* Start thread to detect link periodically for Hot Plug autodetect */
	sys_thread_new("link_detect_thread", link_detect_thread, netif,
			THREAD_STACKSIZE, tskIDLE_PRIORITY);
#endif

	/* set mac address */
	netif->hwaddr_len = NETIF_MAX_HWADDR_LEN;
	for (i = 0; i < NETIF_MAX_HWADDR_LEN; i++)
		netif->hwaddr[i] = mac_ethernet_address[i];

	netif->name[0] = IFNAME0;
  	netif->name[1] = IFNAME1;
	  
	/* initialize based on MAC type */
	return netif_add(netif, 
#if LWIP_IPV4
		(const ip4_addr_t *)ipaddr, netmask, gw,
#endif
		(void*)(uintptr)mac_id,

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
	struct xmac_netif *emac = (struct xmac_netif *)netif->state;
	while (1) {
		/* sleep until there are packets to process
		 * This semaphore is set by the packet receive interrupt
		 * routine.
		 */
		sys_sem_wait(&emac->sem_rx_data_available);

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
static u32 phy_link_detect(FXmac *instance_p, u32 phy_addr)
{
	u16 status;

	/* Read Phy Status register twice to get the confirmation of the current
	 * link status.
	 */

	FXmacPhyRead(instance_p, phy_addr, PHY_STATUS_REG_OFFSET, &status);
	FXmacPhyRead(instance_p, phy_addr, PHY_STATUS_REG_OFFSET, &status);

	if (status & PHY_STAT_LINK_STATUS)
		return 1;
	return 0;
}

static u32 phy_autoneg_status(FXmac *instance_p, u32 phy_addr)
{
	u16 status;

	/* Read Phy Status register twice to get the confirmation of the current
	 * link status.
	 */
	FXmacPhyRead(instance_p, phy_addr, PHY_STATUS_REG_OFFSET, &status);
	FXmacPhyRead(instance_p, phy_addr, PHY_STATUS_REG_OFFSET, &status);

	if (status & PHY_STATUS_AUTONEGOTIATE_COMPLETE)
		return 1;
	return 0;
}



void lwip_port_link_detect(struct netif *netif)
{
	u32 link_speed, phy_link_status;
	struct xmac_netif *xmac_netif_p = (struct xmac_netif *)(netif->state);
	ethernetif *ethernetif_p = (ethernetif *)(xmac_netif_p->state);
	FXmac *instance_p = &ethernetif_p->xmac_ctrl;

	if ((instance_p->is_ready != (u32)FT_COMPONENT_IS_READY) ||
			(eth_link_status == ETH_LINK_UNDEFINED))
		return;

	phy_link_status = phy_link_detect(instance_p, phyaddrforemac);

	if ((eth_link_status == ETH_LINK_UP) && (!phy_link_status))
		eth_link_status = ETH_LINK_DOWN;

	switch (eth_link_status) {
		case ETH_LINK_UNDEFINED:
		case ETH_LINK_UP:
			return;
		case ETH_LINK_DOWN:
			netif_set_link_down(netif);
			eth_link_status = ETH_LINK_NEGOTIATING;
			LWIP_PORT_DEBUG("Ethernet Link down");
			break;
		case ETH_LINK_NEGOTIATING:
			if (phy_link_status && phy_autoneg_status(instance_p, phyaddrforemac)) 
			{
				/* Initiate Phy setup to get link speed */
				netif_set_link_up(netif);
				eth_link_status = ETH_LINK_UP;
				LWIP_PORT_DEBUG("Ethernet Link up");
			}
			break;
	}
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

	ethernetif *mac_netif = (ethernetif *)(netif->state);
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

	ethernetif *mac_netif = (ethernetif *)(netif->state);
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

	ethernetif *mac_netif = (ethernetif *)(netif->state);
	FGmac *instance_p = mac_netif->ethctrl;

	if (instance_p->is_ready != FT_COMPONENT_IS_READY)
	{
		LWIP_PORT_ERROR("instance_p is not ready\n");
		return;
	}

	FGmacStopTrans(instance_p);
}

#endif

/**
* @brief  Returns the current time in milliseconds
*         when LWIP_TIMERS == 1 and NO_SYS == 1
* @param  None
* @retval Time
*/
u32_t sys_now(void)
{
	return GenericGetTick();
}

#ifdef OS_IS_FREERTOS
void link_detect_thread(void *p)
{
	struct netif *netif = (struct netif *) p;

	while (1) {
		/* Call lwip_port_link_detect() every second to detect Ethernet link
		 * change.
		 */
		lwip_port_link_detect(netif);
		vTaskDelay(LINK_DETECT_THREAD_INTERVAL / portTICK_RATE_MS);
	}
}
#endif
