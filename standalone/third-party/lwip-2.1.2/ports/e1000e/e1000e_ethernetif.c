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
 * FilePath: e1000e_ethernetif.c
 * Date: 2025-01-06 19:18:46
 * LastEditTime: 2025-01-06 19:18:46
 * Description:  This file is the function file of the e1000e adaptation to lwip stack.
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huangjin     2025/01/06            first release
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
#include "e1000e_lwip_port.h"
#include "fdebug.h"


#define FE1000E_LWIP_NET_DEBUG_TAG "E1000E_LWIP_NET"
#define FE1000E_LWIP_NET_PRINT_E(format, ...) \
    FT_DEBUG_PRINT_E(FE1000E_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_LWIP_NET_PRINT_I(format, ...) \
    FT_DEBUG_PRINT_I(FE1000E_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_LWIP_NET_PRINT_D(format, ...) \
    FT_DEBUG_PRINT_D(FE1000E_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_LWIP_NET_PRINT_W(format, ...) \
    FT_DEBUG_PRINT_W(FE1000E_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)


#if LWIP_IPV6
#include "lwip/ethip6.h"
#endif

static void e1000e_ethernetif_input(struct netif *netif);

enum lwip_port_link_status e1000e_ethernetif_link_detect(struct netif *netif)
{
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    FE1000ELwipPort *instance_p;
    if (lwip_port == NULL)
    {
        return ETH_LINK_UNDEFINED;
    }
    instance_p = (FE1000ELwipPort *)lwip_port->state;
    if (instance_p->instance.is_ready != FT_COMPONENT_IS_READY)
    {
        return ETH_LINK_UNDEFINED;
    }

    return FE1000EPhyReconnect(lwip_port);
}

void e1000e_ethernetif_debug(struct netif *netif)
{
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    FE1000ELwipPort *instance_p;
    if (lwip_port == NULL)
    {
        FE1000E_LWIP_NET_PRINT_E("lwip_port is an NULL pointer");
        return;
    }
    instance_p = (FE1000ELwipPort *)lwip_port->state;
    if (instance_p->instance.is_ready != FT_COMPONENT_IS_READY)
    {
        FE1000E_LWIP_NET_PRINT_E("The drive is not ready");
        return;
    }

    FE1000EDebugPrint(&instance_p->instance);
}

static void e1000e_ethernetif_start(struct netif *netif)
{
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    if (lwip_port == NULL)
    {
        FE1000E_LWIP_NET_PRINT_E("%s,lwip_port is NULL\n", __FUNCTION__);
        return;
    }
    FE1000ELwipPort *instance_p = (FE1000ELwipPort *)(lwip_port->state);
    FE1000ELwipPortStart(instance_p);
}

static void e1000e_ethernetif_deinit(struct netif *netif)
{
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    if (lwip_port == NULL)
    {
        FE1000E_LWIP_NET_PRINT_E("%s,lwip_port is NULL\n", __FUNCTION__);
        return;
    }

    FE1000ELwipPort *instance_p = (FE1000ELwipPort *)(lwip_port->state);

    FE1000ELwipPortStop(instance_p);
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
    FE1000ELwipPort *instance_p = NULL;
    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    FASSERT(lwip_port != NULL);

    instance_p = (FE1000ELwipPort *)(lwip_port->state);

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    ret = FE1000ELwipPortTx(instance_p, p);

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    if (ret != FT_SUCCESS)
    {
        return ERR_MEM;
    }
    else
    {
        FE1000E_LWIP_NET_PRINT_D("FE1000ELwipPortTx is ok!!!");
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
    FE1000ELwipPort *instance_p = NULL;
    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    FASSERT(lwip_port != NULL);
    instance_p = (FE1000ELwipPort *)(lwip_port->state);

    return FE1000ELwipPortQueueRx(instance_p);
}


static void e1000e_ethernetif_input(struct netif *netif)
{
    struct eth_hdr *ethhdr;
    struct pbuf *p;
    struct LwipPort *e1000e_netif_p = (struct LwipPort *)(netif->state);
    FASSERT(e1000e_netif_p != NULL);
    FE1000ELwipPort *instance_p = NULL;
    instance_p = (FE1000ELwipPort *)(e1000e_netif_p->state);

    SYS_ARCH_DECL_PROTECT(lev);

    while (1)
    {
        SYS_ARCH_PROTECT(lev);
        if (FE1000ELwipPortRxComplete(instance_p))
        {
            SYS_ARCH_UNPROTECT(lev);
            FE1000ELwipPortRx(instance_p);
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
            return;
        }

        /* points to packet payload, which starts with an Ethernet header */
        ethhdr = p->payload;

#if LINK_STATS
        lwip_stats.link.recv++;
#endif /* LINK_STATS */
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
#endif /* PPPOE_SUPPORT */

                /* 处理数据包，调用网络接口的input函数将数据包交给lwip协议栈内核 */
                if (netif->input(p, netif) != ERR_OK)
                {
                    LWIP_DEBUGF(NETIF_DEBUG,
                                ("e1000e_ethernetif_input: IP input error\r\n"));
                    pbuf_free(p);
                    p = NULL;
                }
                break;

            default:
                LWIP_DEBUGF(NETIF_DEBUG, ("e1000e_ethernetif_input: default\r\n"));
                pbuf_free(p);
                p = NULL;
                break;
        }
    }

    return;
}

static void UserConfigConvert(FE1000ELwipPort *instance_p, UserConfig *config_p)
{
    FASSERT(config_p != NULL);
    FASSERT_MSG(config_p->autonegotiation <= 1,
                "config_p->autonegotiation %d is over 1", config_p->autonegotiation);
    FASSERT_MSG(config_p->phy_speed <= FE1000E_PHY_SPEED_10G,
                "config_p->phy_speed %d is over 1000", config_p->phy_speed);
    FASSERT_MSG(config_p->phy_duplex <= FE1000E_PHY_FULL_DUPLEX,
                "config_p->phy_duplex %d is over FE1000E_PHY_FULL_DUPLEX", config_p->phy_duplex);

    FE1000ELwipPortConfig mac_lwip_port_config;

    switch (config_p->mii_interface)
    {
        case LWIP_PORT_INTERFACE_RGMII:
            mac_lwip_port_config.interface = FE1000E_LWIP_PORT_INTERFACE_RGMII;
            break;
        case LWIP_PORT_INTERFACE_SGMII:
            mac_lwip_port_config.interface = FE1000E_LWIP_PORT_INTERFACE_SGMII;
            break;
        case LWIP_PORT_INTERFACE_USX:
            mac_lwip_port_config.interface = FE1000E_LWIP_PORT_INTERFACE_USXGMII;
            break;
        default:
            mac_lwip_port_config.interface = FE1000E_LWIP_PORT_INTERFACE_RGMII;
            break;
    }

    mac_lwip_port_config.instance_id = config_p->mac_instance;
    mac_lwip_port_config.autonegotiation = config_p->autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    mac_lwip_port_config.phy_speed = config_p->phy_speed;   /* FE1000E_PHY_SPEED_XXX */
    mac_lwip_port_config.phy_duplex = config_p->phy_duplex; /* FE1000E_PHY_XXX_DUPLEX */

    instance_p->e1000e_port_config = mac_lwip_port_config;
    instance_p->feature = config_p->capability;
}

static err_t low_level_init(struct netif *netif)
{
    struct LwipPort *lwip_port;
    FE1000ELwipPort *instance_p;
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
    FE1000E_LWIP_NET_PRINT_I("netif->state is %p \r\n", netif->state);

    user_config = (UserConfig *)netif->state;
    if (user_config == NULL)
    {
        FE1000E_LWIP_NET_PRINT_E("UserConfig is NULL");
        mem_free(lwip_port);
        return ERR_MEM;
    }

    instance_p = FE1000ELwipPortGetInstancePointer(user_config->mac_instance);
    if (instance_p == NULL)
    {
        FE1000E_LWIP_NET_PRINT_E("FE1000ELwipPortGetInstancePointer is error\r\n");
        mem_free(lwip_port);
        return ERR_ARG;
    }

    UserConfigConvert(instance_p, user_config);

    for (int i = 0; i < FE1000E_MAX_HARDWARE_ADDRESS_LENGTH; i++)
    {
        instance_p->hwaddr[i] = netif->hwaddr[i];
        instance_p->instance.mac[i] = netif->hwaddr[i];
    }

    ret = FE1000ELwipPortInit(instance_p);
    if (ret != FT_SUCCESS)
    {
        FE1000E_LWIP_NET_PRINT_E("FE1000ELwipPortInit is error\r\n");
        mem_free(lwip_port);
        return ERR_ARG;
    }
    FE1000E_LWIP_NET_PRINT_D("FE1000ELwipPortInit is success\r\n");

    lwip_port->state = (void *)instance_p;
    netif->state = (void *)lwip_port; /* update state */
    instance_p->stack_pointer = lwip_port;

    /* maximum transfer unit */
    if (instance_p->feature & FE1000E_LWIP_PORT_CONFIG_JUMBO)
    {
        netif->mtu = FE1000E_MTU_JUMBO;
    }
    else
    {
        netif->mtu = FE1000E_MTU;
    }

    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

#if LWIP_IPV6 && LWIP_IPV6_MLD
    netif->flags |= NETIF_FLAG_MLD6;
#endif

#if LWIP_IGMP
    netif->flags |= NETIF_FLAG_IGMP;
#endif

    lwip_port->ops.eth_detect = e1000e_ethernetif_link_detect;
    lwip_port->ops.eth_input = e1000e_ethernetif_input;
    lwip_port->ops.eth_deinit = e1000e_ethernetif_deinit;
    lwip_port->ops.eth_start = e1000e_ethernetif_start;
    lwip_port->ops.eth_debug = e1000e_ethernetif_debug;
    FE1000E_LWIP_NET_PRINT_I("ready to leave netif \r\n");
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
 * ethernetif_e1000e_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */

err_t ethernetif_e1000e_init(struct netif *netif)
{
    err_t err;
    LWIP_DEBUGF(NETIF_DEBUG, ("*******start init e1000e eth\n"));

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
    printf("netif_set_igmp_mac_filter !!!!\r\n");
    netif_set_igmp_mac_filter(netif, xmac_filter_update);
#endif

#endif /* LWIP_IPV4 */

    netif->linkoutput = low_level_output;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif

    err = low_level_init(netif);
    if (err != ERR_OK)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_e1000e_init is error\r\n"));
        return err;
    }

    return ERR_OK;
}
