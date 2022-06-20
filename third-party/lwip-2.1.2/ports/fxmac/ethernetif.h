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
 * FilePath: ethernetif.h
 * Date: 2022-04-02 16:43:32
 * LastEditTime: 2022-04-19 21:27:57
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver Who      Date        Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/netif.h"
#include "netif/etharp.h"
#include "lwip/sys.h"
#include "lwip_port.h"
#include "f_printk.h"
#include "fxmac.h"
#include "ethernetif_queue.h"
#include "arch/cc.h"

#if defined (ARMR5) || (__aarch64__) || (ARMA53_32) || (__MICROBLAZE__)
#if defined (USE_JUMBO_FRAMES)
#define XMAC_USE_JUMBO
#endif
#endif

#define MAX_FRAME_SIZE_JUMBO (FXMAC_MTU_JUMBO + FXMAC_HDR_SIZE + FXMAC_TRL_SIZE)

void 	ethernetif_setmac(u32_t index, u8_t *addr);
u8_t*	ethernetif_getmac(u32_t index);
err_t 	ethernetif_init(struct netif *netif);
void 	ethernetif_input(struct netif *netif);

/* ethernetif_hw.c */
void 	ethernetif_error_handler(FXmac * Temac);

/* structure within each netif, encapsulating all information required for
 * using a particular xmac instance
 */
typedef struct {
	FXmac xmac_ctrl;

	/* queue to store overflow packets */
	pq_queue_t *recv_q;
	pq_queue_t *send_q;

	/* pointers to memory holding buffer descriptors (used only with SDMA) */
	void *rx_bdspace;
	void *tx_bdspace;

	unsigned int last_rx_frms_cntr;

} ethernetif;

u32	is_tx_space_available(ethernetif *emac);

void  xmac_process_sent_bds(ethernetif *ethernetif_p, FXmacBdRing *txring);
void xmac_send_handler(void *arg);
FError xmac_sgsend(ethernetif *ethernetif_p, struct pbuf *p);
void xmac_recv_handler(void *arg);
void xmac_error_handler(void *arg, u8 Direction, u32 ErrorWord);
void setup_rx_bds(ethernetif *ethernetif_p, FXmacBdRing *rxring);
void xmac_handle_tx_errors(struct xmac_netif *xmac_netif_p);
void xmac_handle_dma_tx_error(struct xmac_netif *xmac_netif_p);
void xmac_init(ethernetif *ethernetif_p, struct netif *netif);
void xmac_setup_isr (struct xmac_netif *xmac_netif_p);
FError init_dma(struct xmac_netif *xmac_netif_p);
void xmac_start (ethernetif *ethernetif_p);
void free_txrx_pbufs(ethernetif *ethernetif_p);
void free_onlytx_pbufs(ethernetif *ethernetif_p);
void xmac_init_on_error (ethernetif *ethernetif_p, struct netif *netif);
void clean_dma_txdescs(struct xmac_netif *xmac_netif_p);
void resetrx_on_no_rxdata(ethernetif *ethernetif_p);
void reset_dma(struct xmac_netif *xmac_netif_p);

#ifdef __cplusplus
}
#endif

#endif /* __ETHERNETIF_H__ */
