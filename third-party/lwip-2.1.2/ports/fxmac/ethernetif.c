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
 * FilePath: ethernetif.c
 * Date: 2022-07-21 19:18:46
 * LastEditTime: 2022-07-21 19:18:46
 * Description:  This file is the function file of the xmac adaptation to lwip stack.
 *
 * Modify History:
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 * 1.0   wangxiaodong  2022/6/20  first release
 * 2.0   liuzhihong    2022/1/12  restructure
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
#include "fxmac_os.h"
#include "fdebug.h"


#define FXMAC_LWIP_NET_DEBUG_TAG "FXMAC_LWIP_NET"
#define FXMAC_LWIP_NET_PRINT_E(format, ...) FT_DEBUG_PRINT_E(FXMAC_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_LWIP_NET_PRINT_I(format, ...) FT_DEBUG_PRINT_I(FXMAC_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_LWIP_NET_PRINT_D(format, ...) FT_DEBUG_PRINT_D(FXMAC_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_LWIP_NET_PRINT_W(format, ...) FT_DEBUG_PRINT_W(FXMAC_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)


#if LWIP_IPV6
    #include "lwip/ethip6.h"
#endif

static void ethernetif_input(struct netif *netif);

enum lwip_port_link_status ethernetif_link_detect(struct netif *netif)
{
    struct LwipPort *xmac_netif_p = (struct LwipPort *)(netif->state);
    FXmacOs *instance_p;
    if (xmac_netif_p == NULL)
    {
        return ETH_LINK_UNDEFINED;
    }
    instance_p = (FXmacOs *)xmac_netif_p->state;
    if (instance_p->instance.is_ready != FT_COMPONENT_IS_READY)
    {
        return ETH_LINK_UNDEFINED;
    }

    return  FXmacPhyReconnect(xmac_netif_p);
}

static void ethernetif_start(struct netif *netif)
{
    struct LwipPort *xmac_netif_p = (struct LwipPort *)(netif->state);
    if (xmac_netif_p == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("%s,xmac_netif_p is NULL\n", __FUNCTION__);
        return;
    }
    FXmacOs *instance_p = (FXmacOs *)(xmac_netif_p->state);
    FXmacOsStart(instance_p);
}

static void ethernetif_deinit(struct netif *netif)
{
    struct LwipPort *xmac_netif_p = (struct LwipPort *)(netif->state);
    if (xmac_netif_p == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("%s,xmac_netif_p is NULL\n", __FUNCTION__);
        return;
    }

    FXmacOs *instance_p = (FXmacOs *)(xmac_netif_p->state);
    FXmacOsStop(instance_p);
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
    FError ret ;
    FXmacOs *instance_p = NULL;
    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    struct LwipPort *xmac_netif_p = (struct LwipPort *)(netif->state);
    FASSERT(xmac_netif_p != NULL);

    instance_p = (FXmacOs *)(xmac_netif_p->state) ;

    portENTER_CRITICAL();
#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    ret = FXmacOsTx(instance_p, (void *)p);

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
    portEXIT_CRITICAL();

    if (ret != FT_SUCCESS)
    {
        return ERR_MEM;
    }

    return ERR_OK;
}

/*
 * ethernetif_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function FXmacOsRecvHandler() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 */

static void ethernetif_input(struct netif *netif)
{
   struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    
    if(lwip_port == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("%s,lwip_port is NULL\n", __FUNCTION__);
        return;
    }
    FXmacOs *instance_p = (FXmacOs *)(lwip_port->state);
    
    if(instance_p == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("%s,Fxmac instance_p is NULL\n", __FUNCTION__);
        return;
    }
    do
    {
        FXMAC_WRITEREG32(instance_p->instance.config.base_address, FXMAC_IER_OFFSET,FXMAC_IXR_RXCOMPL_MASK);
        FXmacOsRecvHandler(instance_p);
    }while (xSemaphoreTake(lwip_port->sem_rx_data_available,0)  == pdTRUE );
}

static err_t low_level_init(struct netif *netif)
{
    uintptr mac_address = (uintptr)(netif->state);
    struct LwipPort *xmac_netif_p;
    FXmacOs *instance_p;
    FXmac *xmac_p = NULL;
    FError ret;
    u32 dmacrreg;
    FXmacPhyControl xmac_phy_config;
    s32_t status = FT_SUCCESS;

    UserConfig *config_p;

    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    
    xmac_netif_p = mem_malloc(sizeof * xmac_netif_p);
    if (xmac_netif_p == NULL)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("xmac_netif_p init: out of memory\r\n"));
        return ERR_MEM;
    }

    /* obtain config of this emac */
    FXMAC_LWIP_NET_PRINT_I("netif->state is %p \r\n", netif->state);

    config_p = (UserConfig *)netif->state;
    xmac_phy_config.instance_id = config_p->mac_instance;

    switch (config_p->mii_interface)
    {
        case LWIP_PORT_INTERFACE_RGMII:
            xmac_phy_config.interface = FXMAC_OS_INTERFACE_RGMII;
            break;
        case LWIP_PORT_INTERFACE_SGMII:
            xmac_phy_config.interface = FXMAC_OS_INTERFACE_SGMII;
            break;
        default:
            xmac_phy_config.interface = FXMAC_OS_INTERFACE_RGMII;
            break;
    }

    xmac_phy_config.autonegotiation = config_p->autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    xmac_phy_config.phy_speed = config_p->phy_speed;  /* FXMAC_PHY_SPEED_XXX */
    xmac_phy_config.phy_duplex = config_p->phy_duplex; /* FXMAC_PHY_XXX_DUPLEX */

    instance_p = FXmacOsGetInstancePointer(&xmac_phy_config);
    if (instance_p == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("FXmacOsGetInstancePointer is error\r\n");
        return ERR_ARG;
    }

    for (int i = 0; i < 6; i++)
    {
        instance_p->hwaddr[i] = netif->hwaddr[i];
    }

    instance_p->feature = config_p->capability;

    ret = FXmacOsInit(instance_p);
    if (ret != FT_SUCCESS)
    {
        FXMAC_LWIP_NET_PRINT_E("FXmacOsInit is error\r\n");
        return ERR_ARG;
    }

    xmac_netif_p->state = (void *)instance_p;
    netif->state = (void *)xmac_netif_p; /* update state */
    instance_p->stack_pointer = xmac_netif_p;
    instance_p->netif = (void *) netif;


    /* maximum transfer unit */
    if (instance_p->feature & FXMAC_OS_CONFIG_JUMBO)
    {
        netif->mtu = FXMAC_MTU_JUMBO;
    }
    else
    {
        netif->mtu = FXMAC_MTU ;
    }

    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP |
                   NETIF_FLAG_LINK_UP;

#if LWIP_IPV6 && LWIP_IPV6_MLD
    netif->flags |= NETIF_FLAG_MLD6;
#endif

#if LWIP_IGMP
    netif->flags |= NETIF_FLAG_IGMP;
#endif

    xmac_netif_p->ops.eth_detect = ethernetif_link_detect ;
    xmac_netif_p->ops.eth_input = ethernetif_input;
    xmac_netif_p->ops.eth_deinit = ethernetif_deinit;
    xmac_netif_p->ops.eth_start = ethernetif_start;
    FXMAC_LWIP_NET_PRINT_I("Ready to leave netif \r\n");
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
 * ethernetif_xmac_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */

err_t ethernetif_xmac_init(struct netif *netif)
{
    LWIP_DEBUGF(NETIF_DEBUG, ("*******Start init eth\n"));

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
#endif /* LWIP_IPV4 */

    netif->linkoutput = low_level_output;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif

    low_level_init(netif);
    return ERR_OK;
}


