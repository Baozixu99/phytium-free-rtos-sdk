/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: ethernetif.c
 * Date: 2025-01-22 19:18:46
 * LastEditTime: 2025-01-22 19:18:46
 * Description:  This file is the function file of the xmac msg adaptation to lwip stack.
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huangjin     2025/01/22            first release
 */


#include <stdio.h>
#include <string.h>

#include "lwipopts.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/igmp.h"
#include "netif/etharp.h"
#include "lwip_port.h"
#include "fxmac_msg_lwip_port.h"
#include "fdebug.h"


#define FXMAC_MSG_LWIP_NET_DEBUG_TAG "FXMAC_MSG_LWIP_NET"
#define FXMAC_MSG_LWIP_NET_PRINT_E(format, ...) \
    FT_DEBUG_PRINT_E(FXMAC_MSG_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_LWIP_NET_PRINT_I(format, ...) \
    FT_DEBUG_PRINT_I(FXMAC_MSG_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_LWIP_NET_PRINT_D(format, ...) \
    FT_DEBUG_PRINT_D(FXMAC_MSG_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_LWIP_NET_PRINT_W(format, ...) \
    FT_DEBUG_PRINT_W(FXMAC_MSG_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)


#if LWIP_IPV6
#include "lwip/ethip6.h"
#endif

#if LWIP_IGMP
static err_t xmac_filter_update(struct netif *netif, const ip4_addr_t *group,
                                enum netif_mac_filter_action action);
static u8_t xmac_multicast_entry_mask = 0;
#endif

static void ethernetif_input(struct netif *netif);

enum lwip_port_link_status ethernetif_link_detect(struct netif *netif)
{
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    FXmacMsgLwipPort *instance_p;
    if (lwip_port == NULL)
    {
        return ETH_LINK_UNDEFINED;
    }
    instance_p = (FXmacMsgLwipPort *)lwip_port->state;
    if (instance_p->instance.is_ready != FT_COMPONENT_IS_READY)
    {
        return ETH_LINK_UNDEFINED;
    }

    return FXmacMsgPhyReconnect(lwip_port);
}

void ethernetif_debug(struct netif *netif)
{
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    FXmacMsgLwipPort *instance_p;
    if (lwip_port == NULL)
    {
        FXMAC_MSG_LWIP_NET_PRINT_E("lwip_port is an NULL pointer");
        return;
    }
    instance_p = (FXmacMsgLwipPort *)lwip_port->state;
    if (instance_p->instance.is_ready != FT_COMPONENT_IS_READY)
    {
        FXMAC_MSG_LWIP_NET_PRINT_E("The drive is not ready");
        return;
    }

    FXMAC_MSG_LWIP_NET_PRINT_I("ethernetif_debug debug information!");
}

static void ethernetif_start(struct netif *netif)
{
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    if (lwip_port == NULL)
    {
        FXMAC_MSG_LWIP_NET_PRINT_E("%s,lwip_port is NULL\n", __FUNCTION__);
        return;
    }
    FXmacMsgLwipPort *instance_p = (FXmacMsgLwipPort *)(lwip_port->state);
    FXmacMsgLwipPortStart(instance_p);
}

static void ethernetif_deinit(struct netif *netif)
{
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    if (lwip_port == NULL)
    {
        FXMAC_MSG_LWIP_NET_PRINT_E("%s,lwip_port is NULL\n", __FUNCTION__);
        return;
    }

    FXmacMsgLwipPort *instance_p = (FXmacMsgLwipPort *)(lwip_port->state);

    FXmacMsgLwipPortStop(instance_p);
}

/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    FError ret;
    FXmacMsgLwipPort *instance_p = NULL;
    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    FASSERT(lwip_port != NULL);

    instance_p = (FXmacMsgLwipPort *)(lwip_port->state);

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    ret = FXmacMsgLwipPortTx(instance_p, p);

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    if (ret != FT_SUCCESS)
    {
        return ERR_MEM;
    }

    return ERR_OK;
}

/*
 * low_level_input():
 *
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 */
static struct pbuf *low_level_input(struct netif *netif)
{
    FXmacMsgLwipPort *instance_p = NULL;
    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    FASSERT(lwip_port != NULL);
    instance_p = (FXmacMsgLwipPort *)(lwip_port->state);

    return FXmacMsgLwipPortRx(instance_p);
}

/*
 * ethernetif_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 * Returns the number of packets read (max 1 packet on success,
 * 0 if there are no packets)
 *
 */

static void ethernetif_input(struct netif *netif)
{
    struct eth_hdr *ethhdr;
    struct pbuf *p;
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    u32 recv_count = 0;
    if (lwip_port == NULL)
    {
        FXMAC_MSG_LWIP_NET_PRINT_E("%s,lwip_port is NULL\n", __FUNCTION__);
        return;
    }

    FXmacMsgCtrl *instance_p = (FXmacMsgCtrl *)(lwip_port->state);
    if (instance_p == NULL)
    {
        FXMAC_MSG_LWIP_NET_PRINT_E("%s,FXmac Msg instance_p is NULL\n", __FUNCTION__);
        return;
    }

    FXmacMsgLwipPort *fxmac_port = (FXmacMsgLwipPort *)(lwip_port->state);

    SYS_ARCH_DECL_PROTECT(lev);

    while (1)
    {
        SYS_ARCH_PROTECT(lev);
        if (fxmac_port->recv_flg > 0)
        {
            fxmac_port->recv_flg--;
            SYS_ARCH_UNPROTECT(lev);
            /* 接收完成中断触发后关闭了中断，在此处重新打开 */
            FXmacMsgEnableIrq(instance_p, 0, instance_p->mask);
            /* 接收处理函数 */
            FXmacMsgRecvHandler(instance_p->recv_args);

            if (recv_count++ >= FXMAC_MSG_RECV_MAX_COUNT)
            {
                FXMAC_MSG_LWIP_NET_PRINT_W("%s,FXmacMsgRecvHandler call too many times", __FUNCTION__);
                break;
            }
        }
        else
        {
            SYS_ARCH_UNPROTECT(lev);
            break;
        }
    }

    while (1)
    {
        /* move received packet into a new pbuf */
        p = low_level_input(netif);
        /* no packet could be read, silently ignore this */
        if (p == NULL)
        {
            break;
        }

        /* points to packet payload, which starts with an Ethernet header */
        ethhdr = p->payload;
#if LINK_STATS
        lwip_stats.link.recv++;
#endif
        switch (htons(ethhdr->type))
        {
            /* IP or ARP packet? */
            case ETHTYPE_IP:
            case ETHTYPE_ARP:
#if LWIP_IPV6
            /*IPv6 Packet?*/
            case ETHTYPE_IPV6:
#endif
#if PPPOE_SUPPORT
            /* PPPoE packet? */
            case ETHTYPE_PPPOEDISC:
            case ETHTYPE_PPPOE:
#endif
                /* full packet send to tcpip_thread to process */
                if (netif->input(p, netif) != ERR_OK)
                {
                    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\r\n"));
                    pbuf_free(p);
                    p = NULL;
                }
                break;

            default:
                pbuf_free(p);
                p = NULL;
                break;
        }
    }

    return;
}

static void UserConfigConvert(FXmacMsgLwipPort *instance_p, UserConfig *config_p)
{
    FASSERT(config_p != NULL);
    FASSERT_MSG(config_p->autonegotiation <= 1,
                "config_p->autonegotiation %d is over 1", config_p->autonegotiation);
    FASSERT_MSG(config_p->phy_speed <= FXMAC_MSG_PHY_SPEED_10G,
                "config_p->phy_speed %d is over 1000", config_p->phy_speed);
    FASSERT_MSG(config_p->phy_duplex <= FXMAC_MSG_PHY_FULL_DUPLEX,
                "config_p->phy_duplex %d is over FXMAC_PHY_FULL_DUPLEX", config_p->phy_duplex);

    FXmacMsgLwipPortConfig mac_lwip_port_config;

    switch (config_p->mii_interface)
    {
        case LWIP_PORT_INTERFACE_RGMII:
            mac_lwip_port_config.interface = FXMAC_MSG_LWIP_PORT_INTERFACE_RGMII;
            break;
        case LWIP_PORT_INTERFACE_SGMII:
            mac_lwip_port_config.interface = FXMAC_MSG_LWIP_PORT_INTERFACE_SGMII;
            break;
        case LWIP_PORT_INTERFACE_USX:
            mac_lwip_port_config.interface = FXMAC_MSG_LWIP_PORT_INTERFACE_USXGMII;
            break;
        default:
            mac_lwip_port_config.interface = FXMAC_MSG_LWIP_PORT_INTERFACE_RGMII;
            break;
    }

    mac_lwip_port_config.instance_id = config_p->mac_instance;
    mac_lwip_port_config.autonegotiation = config_p->autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    mac_lwip_port_config.phy_speed = config_p->phy_speed; /* FXMAC_MSG_PHY_SPEED_XXX */
    mac_lwip_port_config.phy_duplex = config_p->phy_duplex; /* FXMAC_MSG_PHY_XXX_DUPLEX */

    instance_p->xmac_port_config = mac_lwip_port_config;
    instance_p->feature = config_p->capability;
}

static err_t low_level_init(struct netif *netif)
{
    struct LwipPort *lwip_port;
    FXmacMsgLwipPort *instance_p;
    FError ret;
    UserConfig *user_config;

    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    /* step 1：malloc lwip port object */
    lwip_port = mem_malloc(sizeof *lwip_port);
    if (lwip_port == NULL)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("lwip_port init: out of memory\r\n"));
        return ERR_MEM;
    }

    /* obtain config of this emac */
    FXMAC_MSG_LWIP_NET_PRINT_I("netif->state is %p \r\n", netif->state);

    user_config = (UserConfig *)netif->state;
    if (user_config == NULL)
    {
        FXMAC_MSG_LWIP_NET_PRINT_E("UserConfig is NULL");
        mem_free(lwip_port);
        return ERR_MEM;
    }

    instance_p = FXmacMsgLwipPortGetInstancePointer(user_config->mac_instance);
    if (instance_p == NULL)
    {
        FXMAC_MSG_LWIP_NET_PRINT_E("FXmacMsgLwipPortGetInstancePointer is error\r\n");
        mem_free(lwip_port);
        return ERR_ARG;
    }

    UserConfigConvert(instance_p, user_config);

    for (int i = 0; i < FXMAX_MSG_MAX_HARDWARE_ADDRESS_LENGTH; i++)
    {
        instance_p->hwaddr[i] = netif->hwaddr[i];
    }

    ret = FXmacMsgLwipPortInit(instance_p);
    if (ret != FT_SUCCESS)
    {
        FXMAC_MSG_LWIP_NET_PRINT_E("FXmacMsgLwipPortInit is error\r\n");
        mem_free(lwip_port);
        return ERR_ARG;
    }

    lwip_port->state = (void *)instance_p;
    netif->state = (void *)lwip_port; /* update state */
    instance_p->stack_pointer = lwip_port;

    /* maximum transfer unit */
    if (instance_p->feature & FXMAC_MSG_LWIP_PORT_CONFIG_JUMBO)
    {
        netif->mtu = FXMAC_MSG_MTU_JUMBO;
    }
    else
    {
        netif->mtu = FXMAC_MSG_MTU;
    }

    if (instance_p->feature & FXMAC_MSG_LWIP_PORT_CONFIG_UNICAST_ADDRESS_FILITER)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("Set unicast hash table!!\n"));
        ret = FXmacMsgSetHash(&instance_p->instance, netif->hwaddr);
        if (ret != FT_SUCCESS)
        {
            FXMAC_MSG_LWIP_NET_PRINT_E("Set unicast hash table is error\r\n");
            mem_free(lwip_port);
            return ERR_ARG;
        }
    }

    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

#if LWIP_IPV6 && LWIP_IPV6_MLD
    netif->flags |= NETIF_FLAG_MLD6;
#endif

#if LWIP_IGMP
    netif->flags |= NETIF_FLAG_IGMP;
#endif

    lwip_port->ops.eth_detect = ethernetif_link_detect;
    lwip_port->ops.eth_input = ethernetif_input;
    lwip_port->ops.eth_deinit = ethernetif_deinit;
    lwip_port->ops.eth_start = ethernetif_start;
    lwip_port->ops.eth_debug = ethernetif_debug;
    FXMAC_MSG_LWIP_NET_PRINT_I("ready to leave netif \r\n");
    return ERR_OK;
}

#if !LWIP_ARP
/**
 * This function has to be completed by user in case of ARP OFF.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if ...
 */
static err_t low_level_output_arp_off(struct netif *netif, struct pbuf *q, const ip4_addr_t *ipaddr)
{
    err_t errval;
    errval = ERR_OK;


    return errval;
}
#endif /* LWIP_ARP */

/*
 * ethernetif_xmac_msg_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */

err_t ethernetif_xmac_msg_init(struct netif *netif)
{
    LWIP_DEBUGF(NETIF_DEBUG, ("*******Start init xmac msg*******\r\n"));

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

#if LWIP_IPV4
#if LWIP_ARP || LWIP_ETHERNET
#if LWIP_ARP
    netif->output = etharp_output;
#else
    /* The user should write ist own code in low_level_output_arp_off function */
    netif->output = low_level_output_arp_off;
#endif /* LWIP_ARP */
#endif /* LWIP_ARP || LWIP_ETHERNET */

#if LWIP_IGMP
    netif_set_igmp_mac_filter(netif, xmac_filter_update);
#endif

#endif /* LWIP_IPV4 */

    netif->linkoutput = low_level_output;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif

    return low_level_init(netif);
}


#if LWIP_IGMP
static void xmac_hash_update(struct netif *netif, u8_t *ip_addr, u8_t action)
{
    u8_t multicast_mac_addr[6];
    struct LwipPort *lwip_port;
    FXmacMsgLwipPort *instance_p;

    lwip_port = (struct LwipPort *)(netif->state);
    instance_p = (FXmacMsgLwipPort *)lwip_port->state;
    FXmacMsgBdRing *txring;
    txring = &(FXMAC_MSG_GET_TXRING(instance_p->instance));

    multicast_mac_addr[0] = 0x01;
    multicast_mac_addr[1] = 0x00;
    multicast_mac_addr[2] = 0x5E;
    multicast_mac_addr[3] = ip_addr[1] & 0x7F;
    multicast_mac_addr[4] = ip_addr[2];
    multicast_mac_addr[5] = ip_addr[3];

    /* Wait till all sent packets are acknowledged from HW */
    while (txring->hw_cnt)
    {
        ;
    }

    SYS_ARCH_DECL_PROTECT(lev);

    SYS_ARCH_PROTECT(lev);

    /* Stop Ethernet */
    FXmacMsgStop(&instance_p->instance);

    if (action == IGMP_ADD_MAC_FILTER)
    {
        /* Set Mulitcast mac address in hash table */
        FXmacMsgSetHash(&instance_p->instance, multicast_mac_addr);
    }
    else if (action == IGMP_DEL_MAC_FILTER)
    {
        /* Remove Mulitcast mac address in hash table */
        FXmacMsgDeleteHash(&instance_p->instance, multicast_mac_addr);
    }

    /* Reset DMA */
    FXmacMsgResetDma(instance_p);

    /* Start Ethernet */
    FXmacMsgStart(&instance_p->instance);

    SYS_ARCH_UNPROTECT(lev);
}

static err_t xmac_filter_update(struct netif *netif, const ip4_addr_t *group,
                                enum netif_mac_filter_action action)
{
    u8_t temp_mask;
    unsigned int i;
    u8_t *ip_addr = (u8_t *)group;

    if ((ip_addr[0] < 224) && (ip_addr[0] > 239))
    {
        LWIP_DEBUGF(NETIF_DEBUG,
                    ("%s: The requested MAC address is not a multicast address.\r\n", __func__));
        LWIP_DEBUGF(NETIF_DEBUG, ("Multicast address add operation failure !!\r\n"));

        return ERR_ARG;
    }
    if (action == IGMP_ADD_MAC_FILTER)
    {
        for (i = 0; i < FXMAC_MSG_MAX_MAC_ADDR; i++)
        {
            temp_mask = (0x01) << i;
            if ((xmac_multicast_entry_mask & temp_mask) == temp_mask)
            {
                continue;
            }
            xmac_multicast_entry_mask |= temp_mask;

            /* Update mac address in hash table */
            xmac_hash_update(netif, ip_addr, action);

            LWIP_DEBUGF(NETIF_DEBUG,
                        ("%s: Multicast MAC address successfully added.\r\n", __func__));

            return ERR_OK;
        }
        if (i == FXMAC_MSG_MAX_MAC_ADDR)
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("%s: No multicast address registers left.\r\n", __func__));
            LWIP_DEBUGF(NETIF_DEBUG,
                        ("Multicast MAC address add operation failure !!\r\n"));
        }
        return ERR_MEM;
    }
    else if (action == IGMP_DEL_MAC_FILTER)
    {
        for (i = 0; i < FXMAC_MSG_MAX_MAC_ADDR; i++)
        {
            temp_mask = (0x01) << i;
            if ((xmac_multicast_entry_mask & temp_mask) != temp_mask)
            {
                continue;
            }
            xmac_multicast_entry_mask &= (~temp_mask);

            /* Update mac address in hash table */
            xmac_hash_update(netif, ip_addr, action);

            LWIP_DEBUGF(NETIF_DEBUG,
                        ("%s: Multicast MAC address successfully removed.\r\n", __func__));

            return ERR_OK;
        }
        if (i == FXMAC_MSG_MAX_MAC_ADDR)
        {
            LWIP_DEBUGF(NETIF_DEBUG,
                        ("%s: No multicast address registers present with\r\n", __func__));
            LWIP_DEBUGF(NETIF_DEBUG, ("the requested Multicast MAC address.\r\n"));
            LWIP_DEBUGF(NETIF_DEBUG, ("Multicast MAC address removal failure!!.\r\n"));

            return ERR_MEM;
        }
    }
    return ERR_OK;
}
#endif
