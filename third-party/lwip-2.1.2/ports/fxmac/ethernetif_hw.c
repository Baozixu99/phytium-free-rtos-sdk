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
 * FilePath: ethernetif_hw.c
 * Date: 2022-04-02 16:43:32
 * LastEditTime: 2022-04-19 21:27:57
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver Who      Date        Changes
 * ----- ------     --------    --------------------------------------
 */

#include "fxmac.h"
#include "ethernetif.h"
#include "lwipopts.h"
#include "interrupt.h"

#ifdef OS_IS_FREERTOS
extern long xInsideISR;
#endif

void xmac_init(ethernetif *ethernetif_p, struct netif *netif)
{
	FXmac *xmac_p;
	u32 status = FT_SUCCESS;

	xmac_p = &ethernetif_p->xmac_ctrl;

#ifdef XMAC_USE_JUMBO
	FXmacSetOptions(xmac_p, FXMAC_JUMBO_ENABLE_OPTION);
#endif

#ifdef LWIP_IGMP
	FXmacSetOptions(xmac_p, FXMAC_MULTICAST_OPTION,0);
#endif

	/* set mac address */
	status = FXmacSetMacAddress(xmac_p, (void*)(netif->hwaddr), 1);
	if (status != FT_SUCCESS)
	{
		f_printk("In %s:Emac Mac Address set failed...\r\n",__func__);
	}

	FXmacSelectClk(xmac_p, 1000);
	
}

void xmac_init_on_error(ethernetif *ethernetif_p, struct netif *netif)
{
	FXmac *xmac_p;
	u32 status = FT_SUCCESS;
	xmac_p = &ethernetif_p->xmac_ctrl;

	/* set mac address */
	status = FXmacSetMacAddress(xmac_p, (void*)(netif->hwaddr), 1);
	if (status != FT_SUCCESS) 
	{
		f_printk("In %s:Emac Mac Address set failed...\r\n",__func__);
	}
}

void xmac_setup_isr (struct xmac_netif *xmac_netif_p)
{
	ethernetif *ethernetif_p;
	ethernetif_p = (ethernetif *)(xmac_netif_p->state);

	/* Setup callbacks */
	FXmacSetHandler(&ethernetif_p->xmac_ctrl, FXMAC_HANDLER_DMASEND, xmac_send_handler, xmac_netif_p);
    FXmacSetHandler(&ethernetif_p->xmac_ctrl, FXMAC_HANDLER_DMARECV, xmac_recv_handler, xmac_netif_p);
    FXmacSetHandler(&ethernetif_p->xmac_ctrl, FXMAC_HANDLER_ERROR, xmac_error_handler, xmac_netif_p);

    InterruptSetPriority(ethernetif_p->xmac_ctrl.config.queue_irq_num[0], 0);
    InterruptInstall(ethernetif_p->xmac_ctrl.config.queue_irq_num[0], FXmacIntrHandler, &ethernetif_p->xmac_ctrl, "fxmac");
    InterruptUmask(ethernetif_p->xmac_ctrl.config.queue_irq_num[0]);

}

void xmac_start(ethernetif *ethernetif_p)
{
	/* start the temac */
	FXmacStart(&ethernetif_p->xmac_ctrl);
}

void xmac_restart_transmitter (ethernetif *ethernetif_p)
{
	u32 reg;
	reg = FXMAC_READREG32(ethernetif_p->xmac_ctrl.config.base_address,
						FXMAC_NWCTRL_OFFSET);
	reg = reg & (~FXMAC_NWCTRL_TXEN_MASK);
	FXMAC_WRITEREG32(ethernetif_p->xmac_ctrl.config.base_address,
											FXMAC_NWCTRL_OFFSET, reg);

	reg = FXMAC_READREG32(ethernetif_p->xmac_ctrl.config.base_address,
							FXMAC_NWCTRL_OFFSET);
	reg = reg | (FXMAC_NWCTRL_TXEN_MASK);
	FXMAC_WRITEREG32(ethernetif_p->xmac_ctrl.config.base_address,
											FXMAC_NWCTRL_OFFSET, reg);
}

void xmac_error_handler(void *arg,u8 direction, u32 error_word)
{
	struct xmac_netif *xmac_netif_p;
	ethernetif   *ethernetif_p;
	FXmacBdRing *rxring;
	FXmacBdRing *txring;
#ifdef OS_IS_FREERTOS
	xInsideISR++;
#endif

	xmac_netif_p = (struct xmac_netif *)(arg);
	ethernetif_p = (ethernetif *)(xmac_netif_p->state);
	rxring = &FXMAC_GET_RXRING((ethernetif_p->xmac_ctrl));
	txring = &FXMAC_GET_TXRING((ethernetif_p->xmac_ctrl));

	if (error_word != 0)
	{
		switch (direction)
		{
			case FXMAC_RECV:
			if (error_word & FXMAC_RXSR_HRESPNOK_MASK)
			{
				LWIP_DEBUGF(NETIF_DEBUG, ("Receive DMA error\r\n"));
				xmac_handle_dma_tx_error(xmac_netif_p);
			}
			if (error_word & FXMAC_RXSR_RXOVR_MASK)
			{
				LWIP_DEBUGF(NETIF_DEBUG, ("Receive over run\r\n"));
				xmac_recv_handler(arg);
				setup_rx_bds(ethernetif_p, rxring);
			}
			if (error_word & FXMAC_RXSR_BUFFNA_MASK)
			{
				LWIP_DEBUGF(NETIF_DEBUG, ("Receive buffer not available\r\n"));
				xmac_recv_handler(arg);
				setup_rx_bds(ethernetif_p, rxring);
			}
			break;
			case FXMAC_SEND:
			if (error_word & FXMAC_TXSR_HRESPNOK_MASK)
			{
				LWIP_DEBUGF(NETIF_DEBUG, ("Transmit DMA error\r\n"));
				xmac_handle_dma_tx_error(xmac_netif_p);
			}
			if (error_word & FXMAC_TXSR_URUN_MASK)
			{
				LWIP_DEBUGF(NETIF_DEBUG, ("Transmit under run\r\n"));
				xmac_handle_tx_errors(xmac_netif_p);
			}
			if (error_word & FXMAC_TXSR_BUFEXH_MASK)
			{
				LWIP_DEBUGF(NETIF_DEBUG, ("Transmit buffer exhausted\r\n"));
				xmac_handle_tx_errors(xmac_netif_p);
			}
			if (error_word & FXMAC_TXSR_RXOVR_MASK)
			{
				LWIP_DEBUGF(NETIF_DEBUG, ("Transmit retry excessed limits\r\n"));
				xmac_handle_tx_errors(xmac_netif_p);
			}
			if (error_word & FXMAC_TXSR_FRAMERX_MASK)
			{
				LWIP_DEBUGF(NETIF_DEBUG, ("Transmit collision\r\n"));
				xmac_process_sent_bds(ethernetif_p, txring);
			}
			break;
		}
	}
#ifdef OS_IS_FREERTOS
	xInsideISR--;
#endif
}
