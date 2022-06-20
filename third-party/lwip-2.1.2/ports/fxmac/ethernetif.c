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
 * FilePath: ethernetif.c
 * Date: 2022-04-02 16:43:32
 * LastEditTime: 2022-04-19 21:27:57
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver Who      Date        Changes
 * ----- ------     --------    --------------------------------------
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
#include "ethernetif.h"
#include "lwip_port.h"
#include "ethernetif_queue.h"
#include "fxmac.h"

#if LWIP_IPV6
#include "lwip/ethip6.h"
#endif

/* Define those to better describe your network interface. */
#define IFNAME0 't'
#define IFNAME1 'e'

FXmacConfig *mac_config;
struct netif *NetIf;

#if defined(OS_IS_FREERTOS) && defined(__arm__) && !defined(ARMR5)
int32_t lExpireCounter = 0;
#define RESETRXTIMEOUT 10
#endif

/*
 * this function is always called with interrupts off
 * this function also assumes that there are available BD's
 */
static err_t unbuffered_low_level_output(ethernetif *ethernetif_p, struct pbuf *p)
{
    FError status = 0;

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
    status = xmac_sgsend(ethernetif_p, p);
    if (status != FT_SUCCESS)
    {
#if LINK_STATS
        lwip_stats.link.drop++;
#endif
    }

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

#if LINK_STATS
    lwip_stats.link.xmit++;
#endif /* LINK_STATS */

    return ERR_OK;
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
    SYS_ARCH_DECL_PROTECT(lev);
    err_t err;
    u32 freecnt;
    FXmacBdRing *txring;

    struct xmac_netif *xmac_netif_p = (struct xmac_netif *)(netif->state);
    ethernetif *ethernetif_p = (ethernetif *)(xmac_netif_p->state);

    SYS_ARCH_PROTECT(lev);

    /* check if space is available to send */
    freecnt = is_tx_space_available(ethernetif_p);
    if (freecnt <= 5)
    {
        txring = &(FXMAC_GET_TXRING(ethernetif_p->xmac_ctrl));
        xmac_process_sent_bds(ethernetif_p, txring);
    }

    if (is_tx_space_available(ethernetif_p))
    {
        unbuffered_low_level_output(ethernetif_p, p);
        err = ERR_OK;
    }
    else
    {
#if LINK_STATS
        lwip_stats.link.drop++;
#endif
        f_printk("pack dropped, no space\r\n");
        err = ERR_MEM;
    }

    SYS_ARCH_UNPROTECT(lev);
    return err;
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
    struct xmac_netif *xmac_netif_p = (struct xmac_netif *)(netif->state);
    ethernetif *ethernetif_p = (ethernetif *)(xmac_netif_p->state);
    struct pbuf *p;

    f_printk("pq_qlength(ethernetif_p->recv_q)=%d\n", xmac_pq_qlength(ethernetif_p->recv_q));
    /* see if there is data to process */
    if (xmac_pq_qlength(ethernetif_p->recv_q) == 0)
        return NULL;

    /* return one packet from receive q */
    p = (struct pbuf *)xmac_pq_dequeue(ethernetif_p->recv_q);
    return p;
}

/*
 * ethernetif_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actual transmission of the packet.
 *
 */

static err_t ethernetif_output(struct netif *netif, struct pbuf *p,
                              const ip_addr_t *ipaddr)
{
    /* resolve hardware address, then send (or queue) packet */
    return etharp_output(netif, p, ipaddr);
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

void ethernetif_input(struct netif *netif)
{
    struct eth_hdr *ethhdr;
    struct pbuf *p;
    SYS_ARCH_DECL_PROTECT(lev);

#ifdef OS_IS_FREERTOS
    while (1)
#endif
    {
        /* move received packet into a new pbuf */
        SYS_ARCH_PROTECT(lev);
        p = low_level_input(netif);
        SYS_ARCH_UNPROTECT(lev);

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

#if defined(OS_IS_FREERTOS) && defined(__arm__) && !defined(ARMR5)
void vTimerCallback(TimerHandle_t pxTimer)
{
    /* Do something if the pxTimer parameter is NULL */
    configASSERT(pxTimer);

    lExpireCounter++;
    /* If the timer has expired 100 times then reset RX */
    if (lExpireCounter >= RESETRXTIMEOUT)
    {
        lExpireCounter = 0;
        ethernetif_resetrx_on_no_rxdata(NetIf);
    }
}
#endif

static err_t low_level_init(struct netif *netif)
{
    uintptr mac_address = (uintptr)(netif->state);
    struct xmac_netif *xmac_netif_p;
    ethernetif *ethernetif_p;
    u32 dmacrreg;
    s32_t status = FT_SUCCESS;
    NetIf = netif;

    ethernetif_p = mem_malloc(sizeof *ethernetif_p);
    if (ethernetif_p == NULL)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_p_init: out of memory\r\n"));
        return ERR_MEM;
    }

    xmac_netif_p = mem_malloc(sizeof *xmac_netif_p);
    if (xmac_netif_p == NULL)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_p_init: out of memory\r\n"));
        return ERR_MEM;
    }

    xmac_netif_p->state = (void *)ethernetif_p;
    ethernetif_p->send_q = NULL;
    ethernetif_p->recv_q = xmac_pq_create_queue();
    if (!ethernetif_p->recv_q)
        return ERR_MEM;

        /* maximum transfer unit */
#ifdef XMAC_USE_JUMBO
    netif->mtu = FXMAC_MTU_JUMBO - FXMAC_HDR_SIZE;
#else
    netif->mtu = FXMAC_MTU - FXMAC_HDR_SIZE;
#endif

    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP |
                   NETIF_FLAG_LINK_UP;

#if !NO_SYS
    sys_sem_new(&xmac_netif_p->sem_rx_data_available, 0);
#endif
    /* obtain config of this emac */
    mac_config = (FXmacConfig *)FXmacLookupConfig((unsigned)(uintptr)netif->state);

    status = FXmacCfgInitialize(&ethernetif_p->xmac_ctrl, mac_config);
    if (status != FT_SUCCESS)
    {
        f_printk("In %s:EmacPs Configuration Failed....\r\n", __func__);
    }

    /* initialize the mac */
    xmac_init(ethernetif_p, netif);

    dmacrreg = FXMAC_READREG32(ethernetif_p->xmac_ctrl.config.base_address, FXMAC_DMACR_OFFSET);
    dmacrreg = dmacrreg | (0x00000010);
    FXMAC_WRITEREG32(ethernetif_p->xmac_ctrl.config.base_address, FXMAC_DMACR_OFFSET, dmacrreg);

#if defined(OS_IS_FREERTOS) && defined(__arm__) && !defined(ARMR5)
    /* Freertos tick is 10ms by default; set period to the same */
    xmac_netif_p->xTimer = xTimerCreate("Timer", 10, pdTRUE, (void *)1, vTimerCallback);
    if (xmac_netif_p->xTimer == NULL)
    {
        f_printk("In %s:Timer creation failed....\r\n", __func__);
    }
    else
    {
        if (xTimerStart(xmac_netif_p->xTimer, 0) != pdPASS)
        {
            f_printk("In %s:Timer start failed....\r\n", __func__);
        }
    }
#endif
    xmac_setup_isr(xmac_netif_p);
    init_dma(xmac_netif_p);
    xmac_start(ethernetif_p);

    /* replace the state in netif (currently the emac baseaddress)
     * with the mac instance pointer.
     */
    netif->state = (void *)xmac_netif_p;

    return ERR_OK;
}

void xmac_handle_dma_tx_error(struct xmac_netif *xmac_netif_p)
{
    ethernetif *ethernetif_p;
    s32_t status = FT_SUCCESS;
    u32 dmacrreg;

    SYS_ARCH_DECL_PROTECT(lev);
    SYS_ARCH_PROTECT(lev);

    ethernetif_p = (ethernetif *)(xmac_netif_p->state);
    free_txrx_pbufs(ethernetif_p);
    status = FXmacCfgInitialize(&ethernetif_p->xmac_ctrl, mac_config);

    if (status != FT_SUCCESS)
    {
        f_printk("In %s:EmacPs Configuration Failed....\r\n", __func__);
    }

    /* initialize the mac */
    xmac_init_on_error(ethernetif_p, NetIf);
    dmacrreg = FXMAC_READREG32(ethernetif_p->xmac_ctrl.config.base_address, FXMAC_DMACR_OFFSET);
    dmacrreg = dmacrreg | (0x01000000); /* force_discard_on_err */
    FXMAC_WRITEREG32(ethernetif_p->xmac_ctrl.config.base_address, FXMAC_DMACR_OFFSET, dmacrreg);
    xmac_setup_isr(xmac_netif_p);
    init_dma(xmac_netif_p);
    xmac_start(ethernetif_p);

    SYS_ARCH_UNPROTECT(lev);
}

void xmac_handle_tx_errors(struct xmac_netif *xmac_netif_p)
{
    ethernetif *ethernetif_p;
    u32 netctrlreg;

    SYS_ARCH_DECL_PROTECT(lev);
    SYS_ARCH_PROTECT(lev);
    ethernetif_p = (ethernetif *)(xmac_netif_p->state);
    netctrlreg = FXMAC_READREG32(ethernetif_p->xmac_ctrl.config.base_address,
                                 FXMAC_NWCTRL_OFFSET);
    netctrlreg = netctrlreg & (~FXMAC_NWCTRL_TXEN_MASK);
    FXMAC_WRITEREG32(ethernetif_p->xmac_ctrl.config.base_address,
                     FXMAC_NWCTRL_OFFSET, netctrlreg);
    free_onlytx_pbufs(ethernetif_p);

    clean_dma_txdescs(xmac_netif_p);
    netctrlreg = FXMAC_READREG32(ethernetif_p->xmac_ctrl.config.base_address, FXMAC_NWCTRL_OFFSET);
    netctrlreg = netctrlreg | (FXMAC_NWCTRL_TXEN_MASK);
    FXMAC_WRITEREG32(ethernetif_p->xmac_ctrl.config.base_address, FXMAC_NWCTRL_OFFSET, netctrlreg);
    SYS_ARCH_UNPROTECT(lev);
}

/*
 * ethernetif_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */

err_t ethernetif_init(struct netif *netif)
{
#if LWIP_SNMP
    /* ifType ethernetCsmacd(6) @see RFC1213 */
    netif->link_type = 6;
    /* your link speed here */
    netif->link_speed = ;
    netif->ts = 0;
    netif->ifinoctets = 0;
    netif->ifinucastpkts = 0;
    netif->ifinnucastpkts = 0;
    netif->ifindiscards = 0;
    netif->ifoutoctets = 0;
    netif->ifoutucastpkts = 0;
    netif->ifoutnucastpkts = 0;
    netif->ifoutdiscards = 0;
#endif

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    netif->output = ethernetif_output;
    netif->linkoutput = low_level_output;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif

    low_level_init(netif);
    return ERR_OK;
}

