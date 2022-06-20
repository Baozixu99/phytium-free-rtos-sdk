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
 * FilePath: ethernetif_dma.c
 * Date: 2022-04-02 16:43:32
 * LastEditTime: 2022-04-19 21:27:57
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver Who      Date        Changes
 * ----- ------     --------    --------------------------------------
 */

#include "lwipopts.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/inet_chksum.h"
#include "ethernetif.h"

#include "ft_types.h"
#include "fxmac.h"
#include "parameters.h"
#include "lwip_port.h"
#include "interrupt.h"
#include "fxmac_bd.h"
#include "f_printk.h"
#include "cache.h"

#ifdef __aarch64__
#include "aarch64.h"
#else
#include "cp15.h"
#endif

#ifdef OS_IS_FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"
#endif

#define XMAC_LWIP_CONFIG_N_TX_DESC 64
#define XMAC_LWIP_CONFIG_N_RX_DESC 64

/* Byte alignment of BDs */
#define BD_ALIGNMENT (FXMAC_DMABD_MINIMUM_ALIGNMENT*2)

/* A max of 4 different ethernet interfaces are supported */
static uintptr tx_pbufs_storage[4*XMAC_LWIP_CONFIG_N_TX_DESC];
static uintptr rx_pbufs_storage[4*XMAC_LWIP_CONFIG_N_RX_DESC];

static u32 xmac_intr_num;

/******************************************************************************
 * Each BD is of 8 bytes of size and the BDs (BD chain) need to be  put
 * at uncached memory location. If they are not put at uncached
 * locations, the user needs to flush or invalidate for each BD/packet.
 * However, the flush or invalidate can happen over a cache line which can
 * span multiple BDs. This means a flush or invalidate of one BD can actually
 * flush/invalidate multiple BDs adjacent to the targeted BD.Assuming that
 * the user and hardware both update the BD fields, this operation from user
 * can potentially overwrite the updates done by hardware or user.
 * To avoid this, it is always safe to put the BD chains for Rx and tx side
 * at uncached memory location.
 *
 * The standalone BSP for Cortex A9 implements only primary page tables.
 * Each table entry corresponds to 1 MB of address map. This means, if a memory
 * region has to be made uncached, the minimum granularity will be of 1 MB.
 *
 * The implementation below allocates a 1 MB of u8 array aligned to 1 MB.
 * This ensures that this array is put at 1 MB aligned memory (e.g. 0x1200000)
 * and accupies memory of 1 MB. The init_dma function then changes 1 MB of this
 * region to make it uncached (strongly ordered).
 * This increases the bss section of the program significantly and can be a
 * wastage of memory. The reason beings, BDs will hardly occupy few KBs of
 * memory and the rest of 1 MB of memory will be unused.
 *
 * If a program uses other peripherals that have DMAs/bus masters and need
 * uncached memory, they may also end of following the same approach. This
 * definitely aggravates the memory wastage issue. To avoid all this, the user
 * can create a new 1 MB section in the linker script and reserve it for such
 * use cases that need uncached memory location. They can then have their own
 * memory allocation logic in their application that allocates uncached memory
 * from this 1 MB location. For such a case, changes need to be done in this
 * file and appropriate uncached memory allocated through other means can be
 * used.
 *
 * The present implementation here allocates 1 MB of uncached memory. It
 * reserves of 64 KB of memory for each BD chain. 64 KB of memory means 8192 of
 * BDs for each BD chain which is more than enough for any application.
 * Assuming that both emac0 and emac1 are present, 256 KB of memory is allocated
 * for BDs. The rest 768 KB of memory is just unused.
 *********************************************************************************/

#if defined __aarch64__
u8 bd_space[0x200000] __attribute__ ((aligned (0x200000)));
#else
u8 bd_space[0x100000] __attribute__ ((aligned (0x100000)));
#endif
static volatile u32 bd_space_index = 0;
static volatile u32 bd_space_attr_set = 0;

#ifdef OS_IS_FREERTOS
long xInsideISR = 0;
#endif

#define FXMAC_BD_TO_INDEX(ringptr, bdptr)				\
	(((uintptr)bdptr - (uintptr)(ringptr)->base_bd_addr) / (ringptr)->separation)

/**
 * @name: 
 * @msg:   获取当前bdring 剩余计数
 * @return {*}
 * @param {ethernetif} *ethernetif_p
 */
u32 is_tx_space_available(ethernetif *ethernetif_p)
{
	FXmacBdRing *txring;
	u32 freecnt = 0;

	txring = &(FXMAC_GET_TXRING(ethernetif_p->xmac_ctrl));

	/* tx space is available as long as there are valid BD's */
	freecnt = FXmacBdRingGetFreeCnt(txring);
	return freecnt;
}


/**
 * @name: 
 * @msg: 获取当前用于存放lwip queue 结构体的指针地址，深度为XMAC_LWIP_CONFIG_N_TX_DESC
 * @return {*}
 * @param {ethernetif} *ethernetif_p
 */
static inline u32 get_base_index_tx_buf(ethernetif *ethernetif_p)
{
	return ethernetif_p->xmac_ctrl.config.instance_id *XMAC_LWIP_CONFIG_N_TX_DESC;
}

static inline u32 get_base_index_rx_buf(ethernetif *ethernetif_p)
{
	return ethernetif_p->xmac_ctrl.config.instance_id * XMAC_LWIP_CONFIG_N_RX_DESC;
}

/**
 * @name: 
 * @msg:   释放发送队列q参数
 * @return {*}
 * @param {ethernetif} *ethernetif_p
 * @param {FXmacBdRing} *txring
 */
void xmac_process_sent_bds(ethernetif *ethernetif_p, FXmacBdRing *txring)
{
	FXmacBd *txbdset;
	FXmacBd *curbdpntr;
	u32 n_bds;
	FError status;
	u32 n_pbufs_freed = 0;
	u32 bdindex;
	struct pbuf *p;
	u32 *temp;
	u32 index;

	index = get_base_index_tx_buf(ethernetif_p);

	while (1) 
	{
		/* obtain processed BD's */
		n_bds = FXmacBdRingFromHwTx(txring, XMAC_LWIP_CONFIG_N_TX_DESC, &txbdset);
		if (n_bds == 0)
		{
			return;
		}
		/* free the processed BD's */
		n_pbufs_freed = n_bds;
		curbdpntr = txbdset;
		while (n_pbufs_freed > 0) 
		{
			bdindex = FXMAC_BD_TO_INDEX(txring, curbdpntr);
			temp = (u32 *)curbdpntr;
			*temp = 0; /* Word 0 */
			temp++;
			if (bdindex == (XMAC_LWIP_CONFIG_N_TX_DESC - 1)) 
			{
				*temp = 0xC0000000; /* Word 1 ,used/Wrap – marks last descriptor in transmit buffer descriptor list.*/
			} 
			else 
			{
				*temp = 0x80000000; /* Word 1 , Used – must be zero for GEM to read data to the transmit buffer.*/
			}
			DSB();
			p = (struct pbuf *)tx_pbufs_storage[index + bdindex];
			if (p != NULL) 
			{
				pbuf_free(p);
			}
			tx_pbufs_storage[index + bdindex] = 0;
			curbdpntr = FXmacBdRingNext(txring, curbdpntr);
			n_pbufs_freed--;
			DSB();
		}

		status = FXmacBdRingFree(txring, n_bds, txbdset);
		if (status != FT_SUCCESS) 
		{
			f_printk( ("Failure while freeing in Tx Done ISR\r\n"));
		}
	}
	return;
}

void xmac_send_handler(void *arg)
{
	struct xmac_netif *xmac_netif_p;
	ethernetif   *ethernetif_p;
	FXmacBdRing *txringptr;
	u32 regval;

#ifdef OS_IS_FREERTOS
	xInsideISR++;
#endif

	xmac_netif_p = (struct xmac_netif *)(arg);
	ethernetif_p = (ethernetif *)(xmac_netif_p->state);
	txringptr = &(FXMAC_GET_TXRING(ethernetif_p->xmac_ctrl));
	regval = FXMAC_READREG32(ethernetif_p->xmac_ctrl.config.base_address, FXMAC_TXSR_OFFSET);
	FXMAC_WRITEREG32(ethernetif_p->xmac_ctrl.config.base_address,FXMAC_TXSR_OFFSET, regval); /* 清除中断状态位来停止中断 */
	
	/* If Transmit done interrupt is asserted, process completed BD's */
	xmac_process_sent_bds(ethernetif_p, txringptr);
#ifdef OS_IS_FREERTOS
	xInsideISR--;
#endif
}

FError xmac_sgsend(ethernetif *ethernetif_p, struct pbuf *p)
{
	struct pbuf *q;
	u32 n_pbufs;
	FXmacBd *txbdset, *txbd, *last_txbd = NULL;
	FXmacBd *temp_txbd;
	FError status;
	FXmacBdRing *txring;
	u32 bdindex;
	u32 lev;
	u32 index;
	u32 max_fr_size;

	lev = MFCPSR();
	MTCPSR(lev | 0xC0); /* Mask IRQ and FIQ interrupts in cpsr */

	txring = &(FXMAC_GET_TXRING(ethernetif_p->xmac_ctrl));

	index = get_base_index_tx_buf (ethernetif_p);

	/* first count the number of pbufs */
	for (q = p, n_pbufs = 0; q != NULL; q = q->next)
		n_pbufs++;

	/* obtain as many BD's */
	status = FXmacBdRingAlloc(txring, n_pbufs, &txbdset);
	if (status != FT_SUCCESS) 
	{
		MTCPSR(lev);
		f_printk("sgsend: Error allocating TxBD\r\n");
		return ERR_GENERAL;
	}

	for(q = p, txbd = txbdset; q != NULL; q = q->next)
	{
		bdindex = FXMAC_BD_TO_INDEX(txring, txbd);
		if (tx_pbufs_storage[index + bdindex] != 0) 
		{
			MTCPSR(lev);
			f_printk("PBUFS not available\r\n");
			return ERR_GENERAL;
		}

		/* Send the data from the pbuf to the interface, one pbuf at a
		   time. The size of the data in each pbuf is kept in the ->len
		   variable. */
		FCacheDCacheFlushRange((uintptr)q->payload, (uintptr)q->len);

		FXmacBdSetaddressTx(txbd, (uintptr)q->payload);

#ifdef XMAC_USE_JUMBO
		max_fr_size = FXMAC_MAX_FRAME_SIZE_JUMBO - 18;
#else
		max_fr_size = FXMAC_MAX_FRAME_SIZE - 18;
#endif
		if (q->len > max_fr_size)
			FXmacBdSetLength(txbd, max_fr_size & 0x3FFF);
		else
			FXmacBdSetLength(txbd, q->len & 0x3FFF);

		tx_pbufs_storage[index + bdindex] = (uintptr)q;

		pbuf_ref(q);
		last_txbd = txbd;
		FXmacBdClearLast(txbd);
		txbd = FXmacBdRingNext(txring, txbd);
	}
	FXmacBdSetLast(last_txbd);
	/* For fragmented packets, remember the 1st BD allocated for the 1st
	   packet fragment. The used bit for this BD should be cleared at the end
	   after clearing out used bits for other fragments. For packets without
	   just remember the allocated BD. */
	temp_txbd = txbdset;
	txbd = txbdset;
	txbd = FXmacBdRingNext(txring, txbd);
	q = p->next;
	for(; q != NULL; q = q->next) 
	{
		FXmacBdClearTxUsed(txbd);
		DSB();
		txbd = FXmacBdRingNext(txring, txbd);
	}
	FXmacBdClearTxUsed(temp_txbd);
	DSB();

	status = FXmacBdRingToHw(txring, n_pbufs, txbdset);
	if (status != FT_SUCCESS) 
	{
		MTCPSR(lev);
		f_printk("sgsend: Error submitting TxBD\r\n");
		return ERR_GENERAL;
	}
	/* Start transmit */
	FXMAC_WRITEREG32((ethernetif_p->xmac_ctrl).config.base_address,
	FXMAC_NWCTRL_OFFSET,
	(FXMAC_READREG32((ethernetif_p->xmac_ctrl).config.base_address,
	FXMAC_NWCTRL_OFFSET) | FXMAC_NWCTRL_STARTTX_MASK));

	MTCPSR(lev);
	return status;
}

void setup_rx_bds(ethernetif *ethernetif_p, FXmacBdRing *rxring)
{
	FXmacBd *rxbd;
	FError status;
	struct pbuf *p;
	u32 freebds;
	u32 bdindex;
	u32 *temp;
	u32 index;

	index = get_base_index_rx_buf(ethernetif_p);

	freebds = FXmacBdRingGetFreeCnt(rxring);
	while (freebds > 0) 
	{
		freebds--;

#ifdef XMAC_USE_JUMBO
		p = pbuf_alloc(PBUF_RAW, MAX_FRAME_SIZE_JUMBO, PBUF_POOL);
#else
		p = pbuf_alloc(PBUF_RAW, FXMAC_MAX_FRAME_SIZE, PBUF_POOL);
#endif
		if (!p) 
		{
#if LINK_STATS
			lwip_stats.link.memerr++;
			lwip_stats.link.drop++;
#endif
			f_printk("unable to alloc pbuf in recv_handler\r\n");
			return;
		}
		status = FXmacBdRingAlloc(rxring, 1, &rxbd);
		if (status != FT_SUCCESS) 
		{
			f_printk("setup_rx_bds: Error allocating RxBD\r\n");
			pbuf_free(p);
			return;
		}
		status = FXmacBdRingToHw(rxring, 1, rxbd);
		if (status != FT_SUCCESS) 
		{
			f_printk("Error committing RxBD to hardware: ");
			if (status == FXMAC_ERR_SG_LIST) 
			{
				f_printk("XST_DMA_SG_LIST_ERROR: this function was called out of sequence with FXmacBdRingAlloc()\r\n");
			}
			else 
			{
				f_printk("set of BDs was rejected because the first BD did not have its start-of-packet bit set, or the last BD did not have its end-of-packet bit set, or any one of the BD set has 0 as length value\r\n");
			}

			pbuf_free(p);
			FXmacBdRingUnAlloc(rxring, 1, rxbd);
			return;
		}
#ifdef XMAC_USE_JUMBO
			FCacheDCacheInvalidateRange((uintptr)p->payload, (uintptr)MAX_FRAME_SIZE_JUMBO);	
#else
			FCacheDCacheInvalidateRange((uintptr)p->payload, (uintptr)FXMAC_MAX_FRAME_SIZE);
#endif
		bdindex = FXMAC_BD_TO_INDEX(rxring, rxbd);
		temp = (u32 *)rxbd;
		if (bdindex == (XMAC_LWIP_CONFIG_N_RX_DESC - 1)) 
		{
			*temp = 0x00000002;
		} 
		else 
		{
			*temp = 0;
		}
		temp++;
		*temp = 0;
		DSB();

		FXmacBdSetaddressRx(rxbd, (uintptr)p->payload);
		rx_pbufs_storage[index + bdindex] = (uintptr)p;
	}
}

void xmac_recv_handler(void *arg)
{
	struct pbuf *p;
	FXmacBd *rxbdset, *curbdptr;
	struct xmac_netif *xmac_netif_p;
	ethernetif *ethernetif_p;
	FXmacBdRing *rxring;
	volatile u32 bd_processed;
	u32 rx_bytes, k;
	u32 bdindex;
	u32 regval;
	u32 index;
	u32 gigeversion;

	xmac_netif_p = (struct xmac_netif *)(arg);
	ethernetif_p = (ethernetif *)(xmac_netif_p->state);
	rxring = &FXMAC_GET_RXRING(ethernetif_p->xmac_ctrl);

#ifdef OS_IS_FREERTOS
	xInsideISR++;
#endif

	index = get_base_index_rx_buf (ethernetif_p);

	/* If Reception done interrupt is asserted, call RX call back function
	 to handle the processed BDs and then raise the according flag.*/
	regval = FXMAC_READREG32(ethernetif_p->xmac_ctrl.config.base_address, FXMAC_RXSR_OFFSET);
	FXMAC_WRITEREG32(ethernetif_p->xmac_ctrl.config.base_address, FXMAC_RXSR_OFFSET, regval);

	while(1) 
	{
		bd_processed = FXmacBdRingFromHwRx(rxring, XMAC_LWIP_CONFIG_N_RX_DESC, &rxbdset);
		if (bd_processed <= 0) 
		{
			break;
		}

		for (k = 0, curbdptr=rxbdset; k < bd_processed; k++) 
		{

			bdindex = FXMAC_BD_TO_INDEX(rxring, curbdptr);
			p = (struct pbuf *)rx_pbufs_storage[index + bdindex];

			/*
			 * Adjust the buffer size to the actual number of bytes received.
			 */
#ifdef XMAC_USE_JUMBO
			rx_bytes = FXmacGetRxFrameSize(&ethernetif_p->xmac_ctrl, curbdptr);
#else
			rx_bytes = FXmacBdGetLength(curbdptr);
#endif
			pbuf_realloc(p, rx_bytes);

			/* Invalidate RX frame before queuing to handle
			 * L1 cache prefetch conditions on any architecture.
			 */
			FCacheDCacheInvalidateRange((uintptr)p->payload, rx_bytes);

			/* store it in the receive queue,
			 * where it'll be processed by a different handler
			 */
			if (xmac_pq_enqueue(ethernetif_p->recv_q, (void*)p) < 0) 
			{
#if LINK_STATS
				lwip_stats.link.memerr++;
				lwip_stats.link.drop++;
#endif
				pbuf_free(p);
			}
			curbdptr = FXmacBdRingNext( rxring, curbdptr);
		}
		/* free up the BD's */
		FXmacBdRingFree(rxring, bd_processed, rxbdset);
		setup_rx_bds(ethernetif_p, rxring);
#if !NO_SYS
		sys_sem_signal(&xmac_netif_p->sem_rx_data_available);
#endif
	}

#ifdef OS_IS_FREERTOS
	xInsideISR--;
#endif
	return;
}

void clean_dma_txdescs(struct xmac_netif *xmac_netif_p)
{
	FXmacBd bdtemplate;
	FXmacBdRing *txringptr;
	ethernetif *ethernetif_p = (ethernetif *)(xmac_netif_p->state);

	txringptr = &FXMAC_GET_TXRING((ethernetif_p->xmac_ctrl));
	FXmacBdClear(&bdtemplate);
	FXmacBdSetStatus(&bdtemplate, FXMAC_TXBUF_USED_MASK);

	/* Create the TxBD ring */
	FXmacBdRingCreate(txringptr, (uintptr) ethernetif_p->tx_bdspace,
			(uintptr) ethernetif_p->tx_bdspace, BD_ALIGNMENT,
				 XMAC_LWIP_CONFIG_N_TX_DESC);
	FXmacBdRingClone(txringptr, &bdtemplate, FXMAC_SEND);
}

FError init_dma(struct xmac_netif *xmac_netif_p)
{
	FXmacBd bdtemplate;
	FXmacBdRing *rxringptr, *txringptr;
	FXmacBd *rxbd;
	struct pbuf *p;
	FError status;
	int i;
	u32 bdindex;
	volatile uintptr tempaddress;
	u32 index;
	u32 gigeversion;
	FXmacBd *bdtxterminate;
	FXmacBd *bdrxterminate;
	u32 *temp;

	ethernetif *ethernetif_p = (ethernetif *)(xmac_netif_p->state);

	index = get_base_index_rx_buf(ethernetif_p);
	/*
	 * The BDs need to be allocated in uncached memory. Hence the 1 MB
	 * address range allocated for Bd_Space is made uncached
	 * by setting appropriate attributes in the translation table.
	 * The Bd_Space is aligned to 1MB and has a size of 1 MB. This ensures
	 * a reserved uncached area used only for BDs.
	 */
	if (bd_space_attr_set == 0)
	{
		bd_space_attr_set = 1;
	}

	rxringptr = &FXMAC_GET_RXRING(ethernetif_p->xmac_ctrl);
	txringptr = &FXMAC_GET_TXRING(ethernetif_p->xmac_ctrl);
	f_printk( "rxringptr: 0x%08x\r\n", rxringptr);
	f_printk( "txringptr: 0x%08x\r\n", txringptr);

	/* Allocate 64k for Rx and Tx bds each to take care of extreme cases */
	tempaddress = (uintptr)&(bd_space[bd_space_index]);
	ethernetif_p->rx_bdspace = (void *)tempaddress;
	bd_space_index += 0x10000;
	tempaddress = (uintptr)&(bd_space[bd_space_index]);
	ethernetif_p->tx_bdspace = (void *)tempaddress;
	bd_space_index += 0x10000;

	f_printk( ("rx_bdspace: %p \r\n", ethernetif_p->rx_bdspace));
	f_printk( ("tx_bdspace: %p \r\n", ethernetif_p->tx_bdspace));

	if (!ethernetif_p->rx_bdspace || !ethernetif_p->tx_bdspace) 
	{
		f_printk("%s@%d: Error: Unable to allocate memory for TX/RX buffer descriptors",
				__FILE__, __LINE__);
		return ERR_IF;
	}

	/* Setup RxBD space. */
	FXmacBdClear(&bdtemplate);
	
	/* Create the RxBD ring */
	status = FXmacBdRingCreate(rxringptr, (uintptr) ethernetif_p->rx_bdspace,
				(uintptr) ethernetif_p->rx_bdspace, BD_ALIGNMENT,
				     XMAC_LWIP_CONFIG_N_RX_DESC);

	if(status != FT_SUCCESS)
	{
		f_printk( ("Error setting up RxBD space\r\n"));
		return ERR_IF;
	}

	status = FXmacBdRingClone(rxringptr, &bdtemplate, FXMAC_RECV);
	if(status != FT_SUCCESS)
	{
		f_printk("Error initializing RxBD space\r\n");
		return ERR_IF;
	}

	FXmacBdClear(&bdtemplate);
	FXmacBdSetStatus(&bdtemplate, FXMAC_TXBUF_USED_MASK);

	/* Create the TxBD ring */
	status = FXmacBdRingCreate(txringptr, (uintptr) ethernetif_p->tx_bdspace,
				(uintptr) ethernetif_p->tx_bdspace, BD_ALIGNMENT,
				     XMAC_LWIP_CONFIG_N_TX_DESC);

	if (status != FT_SUCCESS) 
	{
		return ERR_IF;
	}

	/* We reuse the bd template, as the same one will work for both rx and tx. */
	status = FXmacBdRingClone(txringptr, &bdtemplate, FXMAC_SEND);
	if (status != FT_SUCCESS) 
	{
		return ERR_IF;
	}

	/*
	 * Allocate RX descriptors, 1 RxBD at a time.
	 */
	for (i = 0; i < XMAC_LWIP_CONFIG_N_RX_DESC; i++) 
	{
#ifdef XMAC_USE_JUMBO
		p = pbuf_alloc(PBUF_RAW, FXMAC_MAX_FRAME_SIZE_JUMBO, PBUF_POOL);
#else
		p = pbuf_alloc(PBUF_RAW, FXMAC_MAX_FRAME_SIZE, PBUF_POOL);
#endif
		if (!p) 
		{
#if LINK_STATS
			lwip_stats.link.memerr++;
			lwip_stats.link.drop++;
#endif
			f_printk("unable to alloc pbuf in init_dma\r\n");
			return ERR_IF;
		}
		status = FXmacBdRingAlloc(rxringptr, 1, &rxbd);
		if (status != FT_SUCCESS) 
		{
			f_printk( ("init_dma: Error allocating RxBD\r\n"));
			pbuf_free(p);
			return ERR_IF;
		}
		/* Enqueue to HW */
		status = FXmacBdRingToHw(rxringptr, 1, rxbd);
		if (status != FT_SUCCESS) 
		{
			f_printk( ("Error: committing RxBD to HW\r\n"));
			pbuf_free(p);
			FXmacBdRingUnAlloc(rxringptr, 1, rxbd);
			return ERR_IF;
		}

		bdindex = FXMAC_BD_TO_INDEX(rxringptr, rxbd);
		temp = (u32 *)rxbd;
		*temp = 0;
		if (bdindex == (XMAC_LWIP_CONFIG_N_RX_DESC - 1)) 
		{
			*temp = 0x00000002;
		}
		temp++;
		*temp = 0;
		DSB();

#ifdef XMAC_USE_JUMBO
		FCacheDCacheInvalidateRange((uintptr)p->payload, (uintptr)MAX_FRAME_SIZE_JUMBO);
#else
		FCacheDCacheInvalidateRange((uintptr)p->payload, (uintptr)FXMAC_MAX_FRAME_SIZE);
#endif
		FXmacBdSetaddressRx(rxbd, (uintptr)p->payload);

		rx_pbufs_storage[index + bdindex] = (uintptr)p;
	}

	FXmacSetQueuePtr(&(ethernetif_p->xmac_ctrl), ethernetif_p->xmac_ctrl.tx_bd_queue.bdring.base_bd_addr, 0, (u16)FXMAC_SEND);
	FXmacSetQueuePtr(&(ethernetif_p->xmac_ctrl), ethernetif_p->xmac_ctrl.rx_bd_queue.bdring.base_bd_addr, 0, (u16)FXMAC_RECV);

	xmac_intr_num = ethernetif_p->xmac_ctrl.config.queue_irq_num[0];
	return 0;
}

void free_txrx_pbufs(ethernetif *ethernetif_p)
{
	u32 index;
	u32 index1;
	struct pbuf *p;

	index1 = get_base_index_tx_buf(ethernetif_p);

	for (index = index1; index < (index1 + XMAC_LWIP_CONFIG_N_TX_DESC); index++)
	{
		if(tx_pbufs_storage[index] != 0)
		{
			p = (struct pbuf *)tx_pbufs_storage[index];
			pbuf_free(p);
			tx_pbufs_storage[index] = 0;
		}
	}

	for (index = index1; index < (index1 + XMAC_LWIP_CONFIG_N_TX_DESC); index++)
	{
		p = (struct pbuf *)rx_pbufs_storage[index];
		pbuf_free(p);

	}
}

void free_onlytx_pbufs(ethernetif *ethernetif_p)
{
	u32 index;
	u32 index1;
	struct pbuf *p;

	index1 = get_base_index_tx_buf(ethernetif_p);
	for (index = index1; index < (index1 + XMAC_LWIP_CONFIG_N_TX_DESC); index++)
	{
		if (tx_pbufs_storage[index] != 0)
		{
			p = (struct pbuf *)tx_pbufs_storage[index];
			pbuf_free(p);
			tx_pbufs_storage[index] = 0;
		}
	}
}

/* reset Tx and Rx DMA pointers */
void reset_dma(struct xmac_netif *xmac_netif_p)
{
	u8 txqueuenum;
	u32 gigeversion;
	ethernetif *ethernetif_p = (ethernetif *)(xmac_netif_p->state);
	FXmacBdRing *txringptr = &FXMAC_GET_TXRING(ethernetif_p->xmac_ctrl);
	FXmacBdRing *rxringptr = &FXMAC_GET_RXRING(ethernetif_p->xmac_ctrl);

	FXmacBdringPtrReset(txringptr, ethernetif_p->tx_bdspace);
	FXmacBdringPtrReset(rxringptr, ethernetif_p->rx_bdspace);

	FXmacSetQueuePtr(&(ethernetif_p->xmac_ctrl), ethernetif_p->xmac_ctrl.tx_bd_queue.bdring.base_bd_addr, 0, (u16)FXMAC_SEND);
	FXmacSetQueuePtr(&(ethernetif_p->xmac_ctrl), ethernetif_p->xmac_ctrl.rx_bd_queue.bdring.base_bd_addr, 0, (u16)FXMAC_RECV);

}

void xmac_disable_intr(void)
{
	InterruptMask(xmac_intr_num);
}

void xmac_enable_intr(void)
{
	InterruptUmask(xmac_intr_num);
}
