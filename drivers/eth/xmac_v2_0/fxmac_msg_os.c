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
 * FilePath: fxmac_msg_os.c
 * Date: 2025-04-14 16:33:13
 * LastEditTime: 2025-04-14 16:33:13
 * Description:  This file is for xmac v2 driver.Functions in this file are the minimum required functions for drivers.
 *
 * Modify History:
 *  Ver   Who        Date                   Changes
 * ----- ------    --------     --------------------------------------
 *  1.0  huangjin  2025/06/24            first release
 */

#include "fparameters.h"
#include "fassert.h"
#include "fdebug.h"
#include "fcache.h"
#include "fcpu_info.h"
#include "faarch.h"

#include "fxmac_msg.h"
#include "fxmac_msg_bd.h"
#include "fxmac_msg_bdring.h"
#include "fxmac_msg_phy.h"
#include "fxmac_msg_hw.h"
#include "fxmac_msg_common.h"
#include "eth_ieee_reg.h"

#include "lwip_port.h"
#include "fxmac_msg_os.h"
#include "netif/etharp.h"

#include "FreeRTOS.h"
#include "semphr.h"


#define OS_MAC_DEBUG_TAG "OS_XMAC_MSG"
#define FXMAC_MSG_OS_PRINT_E(format, ...) FT_DEBUG_PRINT_E(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_OS_PRINT_I(format, ...) FT_DEBUG_PRINT_I(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_OS_PRINT_D(format, ...) FT_DEBUG_PRINT_D(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_OS_PRINT_W(format, ...) FT_DEBUG_PRINT_W(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)

#define FXMAC_MSG_BD_TO_INDEX(ringptr, bdptr) \
    (((uintptr)bdptr - (uintptr)(ringptr)->base_bd_addr) / (ringptr)->separation)

static void FXmacMsgInitOnError(FXmacMsgOs *instance_p);
static void FXmacMsgSetupIsr(FXmacMsgOs *instance_p);
extern void sys_sem_signal(sys_sem_t *sem);
static FXmacMsgOs fxmac_os_instace[FXMAC_MSG_NUM] =
{
        [FXMAC0_MSG_ID] = {0},
#if defined(FXMAC1_MSG_ID)        
        [FXMAC1_MSG_ID] = {0},
#endif        
#if defined(FXMAC2_MSG_ID)
        [FXMAC_MSG2_ID] = {0},
#endif
#if defined(FXMAC3_MSG_ID)
        [FXMAC3_MSG_ID] = {0},
#endif
};

int isr_calling_flg = 0;

/* queue */

void FXmacQueueInit(PqQueue *q)
{
    FASSERT(q != NULL);
    q->head = q->tail = q->len = 0;
}

int FXmacPqEnqueue(PqQueue *q, void *p)
{
    if (q->len == PQ_QUEUE_SIZE)
    {
        return -1;
    }

    q->data[q->head] = (uintptr)p;
    q->head = (q->head + 1) % PQ_QUEUE_SIZE;
    q->len++;

    return 0;
}

void *FXmacPqDequeue(PqQueue *q)
{
    int ptail;

    if (q->len == 0)
    {
        return NULL;
    }

    ptail = q->tail;
    q->tail = (q->tail + 1) % PQ_QUEUE_SIZE;
    q->len--;

    return (void *)q->data[ptail];
}

int FXmacPqQlength(PqQueue *q)
{
    return q->len;
}

/* dma */

/**
 * @name:  IsTxSpaceAvailable
 * @msg:   获取当前bdring 剩余计数
 * @param {ethernetif} *ethernetif_p
 * @return {*} 返回
 */
static u32 IsTxSpaceAvailable(FXmacMsgOs *instance_p)
{
    FXmacMsgBdRing *txring;
    u32 freecnt = 0;
    FASSERT(instance_p != NULL);

    txring = &(FXMAC_MSG_GET_TXRING(instance_p->instance));

    /* tx space is available as long as there are valid BD's */
    freecnt = FXMAC_MSG_BD_RING_GET_FREE_CNT(txring);
    return freecnt;
}

/**
 * @name: FXmacMsgProcessSentBds
 * @msg:   释放发送队列q参数
 * @return {*}
 * @param {ethernetif} *ethernetif_p
 * @param {FXmacMsgBdRing} *txring
 * @param {u32} cnt
 */
void FXmacMsgProcessSentBds(FXmacMsgOs *instance_p, FXmacMsgBdRing *txring, u32 cnt)
{
    FXmacMsgBd *txbdset;
    FXmacMsgBd *curbdpntr;
    u32 n_bds;
    FError status;
    u32 n_pbufs_freed = 0;
    u32 bdindex;
    struct pbuf *p;
    u32 *temp;

    /* obtain processed BD's */
    n_bds = FXmacMsgBdRingFromHwTx(txring,cnt,&txbdset);
    if (n_bds == 0)
    {
        return;
    }
    /* free the processed BD's */
    n_pbufs_freed = n_bds;
    curbdpntr = txbdset;
    while (n_pbufs_freed > 0)
    {
        bdindex = FXMAC_MSG_BD_TO_INDEX(txring, curbdpntr);
        temp = (u32 *)curbdpntr;
        *temp = 0; /* Word 0 */
        temp++;
        if (bdindex == (FXMAC_MSG_TX_PBUFS_LENGTH - 1))
        {
            *temp = 0xC0000000; /* Word 1 ,used/Wrap – marks last descriptor in transmit buffer descriptor list.*/
        }
        else
        {
            *temp = 0x80000000; /* Word 1 , Used – must be zero for GEM to read data to the transmit buffer.*/
        }
        DSB();

        p = (struct pbuf *)instance_p->buffer.tx_pbufs_storage[bdindex];

        if (p != NULL)
        {
            pbuf_free(p);
        }
        instance_p->buffer.tx_pbufs_storage[bdindex] = (uintptr)NULL;
        curbdpntr = FXMAC_MSG_BD_RING_NEXT(txring, curbdpntr);
        n_pbufs_freed--;
        DSB();
    }

    status = FXmacMsgBdRingFree(txring, n_bds, txbdset);
    if (status != FT_SUCCESS)
    {
        FXMAC_MSG_OS_PRINT_I("Failure while freeing in Tx Done ISR.");
    }

    return;
}

void FXmacMsgSendHandler(void *arg)
{
    FXmacMsgOs *instance_p;
    instance_p = (FXmacMsgOs *)arg;
    FXmacMsgCtrl *xmac_p = &instance_p->instance;
    /* 关闭发送完成中断 */
    FXmacMsgDisableIrq(xmac_p, 0, FXMAC_MSG_INT_TX_COMPLETE);
}

FError FXmacMsgSgsend(FXmacMsgOs *instance_p, struct pbuf *p)
{
    struct pbuf *q;
    u32 n_pbufs;
    FXmacMsgBd *txbdset, *txbd, *last_txbd = NULL;
    FXmacMsgBd *temp_txbd;
    FError status;
    FXmacMsgBdRing *txring;
    u32 bdindex = 0;
    u32 max_fr_size;

    txring = &(FXMAC_MSG_GET_TXRING(instance_p->instance));

    /* first count the number of pbufs */
    for (q = p, n_pbufs = 0; q != NULL; q = q->next)
    {
        n_pbufs++;
    }

    /* obtain as many BD's */
    status = FXmacMsgBdRingAlloc(txring, n_pbufs, &txbdset);
    if (status != FT_SUCCESS)
    {
        FXMAC_MSG_OS_PRINT_E("sgsend: Error allocating TxBD.");
        return ERR_GENERAL;
    }

    for (q = p, txbd = txbdset; q != NULL; q = q->next)
    {
        bdindex = FXMAC_MSG_BD_TO_INDEX(txring, txbd);

        if (instance_p->buffer.tx_pbufs_storage[bdindex])
        {
            FXMAC_MSG_OS_PRINT_I("txbd %p, txring->base_bd_addr %p", txbd, txring->base_bd_addr);
            FXMAC_MSG_OS_PRINT_I("PBUFS not available bdindex is %d ", bdindex);
            FXMAC_MSG_OS_PRINT_I("instance_p->buffer.tx_pbufs_storage[bdindex] %p ", instance_p->buffer.tx_pbufs_storage[bdindex]);
            return ERR_GENERAL;
        }

        /* Send the data from the pbuf to the interface, one pbuf at a
           time. The size of the data in each pbuf is kept in the ->len
           variable. */
        FCacheDCacheFlushRange((uintptr)q->payload, (uintptr)q->len);
        FXMAC_MSG_BD_SET_ADDRESS_TX(txbd, (uintptr)q->payload);

        if (instance_p->feature & FXMAC_MSG_OS_CONFIG_JUMBO)
        {
            max_fr_size = FXMAC_MSG_MAX_FRAME_SIZE_JUMBO;
        }
        else
        {
            max_fr_size = FXMAC_MSG_MAX_FRAME_SIZE;
        }

        if (q->len > max_fr_size)
        {
            FXMAC_MSG_OS_PRINT_D("FreeRTOS max_fr_size = %d", max_fr_size);
            FXMAC_MSG_BD_SET_LENGTH(txbd, max_fr_size & 0x3FFF);
        }
        else
        {
            FXMAC_MSG_OS_PRINT_D("FreeRTOS q->len = %d", q->len);
            FXMAC_MSG_BD_SET_LENGTH(txbd, q->len & 0x3FFF);
        }

        instance_p->buffer.tx_pbufs_storage[bdindex] = (uintptr)q;

        pbuf_ref(q);
        last_txbd = txbd;
        FXMAC_MSG_BD_CLEAR_LAST(txbd);
        txbd = FXMAC_MSG_BD_RING_NEXT(txring, txbd);
    }
    FXMAC_MSG_BD_SET_LAST(last_txbd);

    /* 检查描述符 */
    FXMAC_MSG_OS_PRINT_D("FreeRTOS last_txbd word0 = 0x%x, word1 = 0x%x", (*last_txbd)[0], (*last_txbd)[1]);    

    /* 获取最后可用描述符的下一位编号 */
    bdindex = FXMAC_MSG_BD_TO_INDEX(txring, txbd);

    /* For fragmented packets, remember the 1st BD allocated for the 1st
       packet fragment. The used bit for this BD should be cleared at the end
       after clearing out used bits for other fragments. For packets without
       just remember the allocated BD. */
    temp_txbd = txbdset;
    txbd = txbdset;
    txbd = FXMAC_MSG_BD_RING_NEXT(txring, txbd);
    q = p->next;
    for (; q != NULL; q = q->next)
    {
        FXMAC_MSG_BD_CLEAR_TX_USED(txbd);
        DSB();
        txbd = FXMAC_MSG_BD_RING_NEXT(txring, txbd);
    }
    FXMAC_MSG_BD_CLEAR_TX_USED(temp_txbd);
    DSB();

    /* 更新完USED之后的描述符 */
    FXMAC_MSG_OS_PRINT_D("FreeRTOS Update USED temp_txbd word0 = 0x%x, word1 = 0x%x", (*temp_txbd)[0], (*temp_txbd)[1]);    

    status = FXmacMsgBdRingToHw(txring, n_pbufs, txbdset);
    if (status != FT_SUCCESS)
    {
        FXMAC_MSG_OS_PRINT_I("sgsend: Error submitting TxBD.");
        return ERR_GENERAL;
    }

    /* Start transmit */
    DSB();
    FXMAC_MSG_WRITE((&instance_p->instance), FXMAC_MSG_TX_PTR(0), bdindex);    
    return status;
}

void SetupRxBds(FXmacMsgOs *instance_p, FXmacMsgBdRing *rxring)
{
    FXmacMsgBd *rxbd;
    FError status;
    struct pbuf *p;
    u32 freebds;
    u32 bdindex;
    u32 *temp;
    freebds = FXMAC_MSG_BD_RING_GET_FREE_CNT(rxring);
    while (freebds > 0)
    {
        freebds--;

        if (instance_p->feature & FXMAC_MSG_OS_CONFIG_JUMBO)
        {
            p = pbuf_alloc(PBUF_RAW, FXMAC_MSG_MAX_FRAME_SIZE_JUMBO, PBUF_RAM);
        }
        else
        {
            p = pbuf_alloc(PBUF_RAW, FXMAC_MSG_MAX_FRAME_SIZE, PBUF_POOL);
        }

        if (!p)
        {
#if LINK_STATS
            lwip_stats.link.memerr++;
            lwip_stats.link.drop++;
#endif
            FXMAC_MSG_OS_PRINT_I("Unable to alloc pbuf in recv_handler.");
            return;
        }
        status = FXmacMsgBdRingAlloc(rxring, 1, &rxbd);
        if (status != FT_SUCCESS)
        {
            FXMAC_MSG_OS_PRINT_I("SetupRxBds: Error allocating RxBD.");
            pbuf_free(p);
            return;
        }
        status = FXmacMsgBdRingToHw(rxring, 1, rxbd);
        if (status != FT_SUCCESS)
        {
            FXMAC_MSG_OS_PRINT_I("Error committing RxBD to hardware: ");
            if (status == FXMAC_MSG_ERR_SG_LIST)
            {
                FXMAC_MSG_OS_PRINT_I("XST_DMA_SG_LIST_ERROR: this function was called out of sequence with FXmacMsgBdRingAlloc().");
            }
            else
            {
                FXMAC_MSG_OS_PRINT_I("Set of BDs was rejected because the first BD did not have its start-of-packet bit set, or the last BD did not have its end-of-packet bit set, or any one of the BD set has 0 as length value.");
            }

            pbuf_free(p);
            FXmacMsgBdRingUnAlloc(rxring, 1, rxbd);
            return;
        }

        if (instance_p->feature & FXMAC_MSG_OS_CONFIG_JUMBO)
        {
            FCacheDCacheInvalidateRange((uintptr)p->payload, (uintptr)MAX_FRAME_SIZE_JUMBO);
        }
        else
        {
            FCacheDCacheInvalidateRange((uintptr)p->payload, (uintptr)FXMAC_MSG_MAX_FRAME_SIZE);
        }

        bdindex = FXMAC_MSG_BD_TO_INDEX(rxring, rxbd);
        temp = (u32 *)rxbd;
        if (bdindex == (FXMAC_MSG_RX_PBUFS_LENGTH - 1))
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

        FXMAC_MSG_BD_SET_ADDRESS_RX(rxbd, (uintptr)p->payload);
        instance_p->buffer.rx_pbufs_storage[bdindex] = (uintptr)p;
    }
}
void FXmacRecvSemaphoreHandler(void *arg)
{
    struct LwipPort *xmac_netif_p;
    FXmacMsgOs *instance_p;
    FXmacMsgCtrl *xmac_p = NULL;
    
    instance_p = (FXmacMsgOs *)arg;
    xmac_netif_p = (struct LwipPort *)instance_p->stack_pointer;
    xmac_p = &instance_p->instance;
    FXmacMsgDisableIrq(xmac_p, 0, FXMAC_MSG_INT_RX_COMPLETE);
    sys_sem_signal(&(xmac_netif_p->sem_rx_data_available));
}


/**
 * @name: FXmacRecvHandler
 * @msg: handle dma packets and put these packets to lwip stack to process
 * @note: 
 * @param {void} *arg
*  @return {*}
 */
void FXmacRecvHandler(void *arg)
{
    struct pbuf *p;
    FXmacMsgBd *rxbdset, *curbdptr;
    FXmacMsgBd *rxtailbdptr = NULL;
    FXmacMsgBdRing *rxring;
    volatile u32 bd_processed;
    u32 rx_bytes, k;
    u32 bdindex = 0;
    u32 rx_queue_len;
    u32 rx_tail_bd_index = 0;
    FXmacMsgOs *instance_p;
    FASSERT(arg != NULL);

    instance_p = (FXmacMsgOs *)arg;
    rxring = &FXMAC_MSG_GET_RXRING(instance_p->instance);

    while (1)
    {
        bd_processed = FXmacMsgBdRingFromHwRx(rxring, FXMAC_MSG_RX_PBUFS_LENGTH, &rxbdset);
        if (bd_processed <= 0)
        {
            break;
        }

        for (k = 0, curbdptr = rxbdset; k < bd_processed; k++)
        {
            bdindex = FXMAC_MSG_BD_TO_INDEX(rxring, curbdptr);
            p = (struct pbuf *)instance_p->buffer.rx_pbufs_storage[bdindex];
            /*
             * Adjust the buffer size to the actual number of bytes received.
             */
            if (instance_p->feature & FXMAC_MSG_OS_CONFIG_JUMBO)
            {
                rx_bytes = FXMAC_MSG_GET_RX_FRAME_SIZE(curbdptr);
            }
            else
            {
                rx_bytes = FXMAC_MSG_BD_GET_LENGTH(curbdptr);
            }
            pbuf_realloc(p, rx_bytes);

            /* Invalidate RX frame before queuing to handle
             * L1 cache prefetch conditions on any architecture.
             */
            FCacheDCacheInvalidateRange((uintptr)p->payload, rx_bytes);

            /* store it in the receive queue,
             * where it'll be processed by a different handler
             */
            if (FXmacPqEnqueue(&instance_p->recv_q, (void *)p) < 0)
            {
#if LINK_STATS
                lwip_stats.link.memerr++;
                lwip_stats.link.drop++;
#endif
                pbuf_free(p);
            }
            instance_p->buffer.rx_pbufs_storage[bdindex] = (uintptr)NULL;
            curbdptr = FXMAC_MSG_BD_RING_NEXT(rxring, curbdptr);
            rxtailbdptr = curbdptr;
        }

        /* free up the BD's */
        FXmacMsgBdRingFree(rxring, bd_processed, rxbdset);
        SetupRxBds(instance_p, rxring);

        rx_queue_len = FXmacPqQlength(&instance_p->recv_q);
        while (rx_queue_len)
        {
            /* return one packet from receive q */
            p = (struct pbuf *)FXmacPqDequeue(&instance_p->recv_q);
            FXmacMsgOsRx(instance_p,(void *)p); 
            rx_queue_len--;
        }
    }

    rx_tail_bd_index = FXMAC_MSG_BD_TO_INDEX(rxring, rxtailbdptr);
    DSB();
    FXMAC_MSG_WRITE((&instance_p->instance), FXMAC_MSG_RX_PTR(0), rx_tail_bd_index);
       
    return;
}

void FXmacMsgOsRecvHandler(FXmacMsgOs *instance_p)
{
    FXmacRecvHandler(instance_p);
}

void CleanDmaTxdescs(FXmacMsgOs *instance_p)
{
    FXmacMsgBd bdtemplate;
    FXmacMsgBdRing *txringptr;

    txringptr = &FXMAC_MSG_GET_TXRING((instance_p->instance));
    FXMAC_MSG_BD_CLEAR(&bdtemplate);
    FXMAC_MSG_BD_SET_STATUS(&bdtemplate, FXMAC_MSG_TXBUF_USED_MASK);

    FXmacMsgBdRingCreate(txringptr, (uintptr)instance_p->buffer.tx_bdspace,
                      (uintptr)instance_p->buffer.tx_bdspace, BD_ALIGNMENT,
                      sizeof(instance_p->buffer.tx_bdspace));

    FXmacMsgBdRingClone(txringptr, &bdtemplate, FXMAC_MSG_SEND);
}

FError FXmacMsgInitDma(FXmacMsgOs *instance_p)
{
    FXmacMsgBd bdtemplate;
    FXmacMsgBdRing *rxringptr, *txringptr;
    FXmacMsgBd *rxbd;
    struct pbuf *p;
    FError status;
    int i;
    u32 bdindex;
    volatile uintptr tempaddress;
    FXmacMsgBd *bdtxterminate;
    FXmacMsgBd *bdrxterminate;
    u32 *temp;


    /*
     * The BDs need to be allocated in uncached memory. Hence the 1 MB
     * address range allocated for Bd_Space is made uncached
     * by setting appropriate attributes in the translation table.
     * The Bd_Space is aligned to 1MB and has a size of 1 MB. This ensures
     * a reserved uncached area used only for BDs.
     */

    rxringptr = &FXMAC_MSG_GET_RXRING(instance_p->instance);
    txringptr = &FXMAC_MSG_GET_TXRING(instance_p->instance);
    FXMAC_MSG_OS_PRINT_I("rxringptr: 0x%08x", rxringptr);
    FXMAC_MSG_OS_PRINT_I("txringptr: 0x%08x", txringptr);

    FXMAC_MSG_OS_PRINT_I("rx_bdspace: %p ", instance_p->buffer.rx_bdspace);
    FXMAC_MSG_OS_PRINT_I("tx_bdspace: %p ", instance_p->buffer.tx_bdspace);

    /* Setup RxBD space. */
    FXMAC_MSG_BD_CLEAR(&bdtemplate);

    /* Create the RxBD ring */
    status = FXmacMsgBdRingCreate(rxringptr, (uintptr)instance_p->buffer.rx_bdspace,
                               (uintptr)instance_p->buffer.rx_bdspace, BD_ALIGNMENT,
                               FXMAC_MSG_RX_PBUFS_LENGTH);

    if (status != FT_SUCCESS)
    {
        FXMAC_MSG_OS_PRINT_I("Error setting up RxBD space.");
        return ERR_IF;
    }

    status = FXmacMsgBdRingClone(rxringptr, &bdtemplate, FXMAC_MSG_RECV);
    if (status != FT_SUCCESS)
    {
        FXMAC_MSG_OS_PRINT_I("Error initializing RxBD space.");
        return ERR_IF;
    }

    FXMAC_MSG_BD_CLEAR(&bdtemplate);
    FXMAC_MSG_BD_SET_STATUS(&bdtemplate, FXMAC_MSG_TXBUF_USED_MASK);

    /* Create the TxBD ring */
    status = FXmacMsgBdRingCreate(txringptr, (uintptr)instance_p->buffer.tx_bdspace,
                               (uintptr)instance_p->buffer.tx_bdspace, BD_ALIGNMENT,
                               FXMAC_MSG_TX_PBUFS_LENGTH);

    if (status != FT_SUCCESS)
    {
        return ERR_IF;
    }

    /* We reuse the bd template, as the same one will work for both rx and tx. */
    status = FXmacMsgBdRingClone(txringptr, &bdtemplate, FXMAC_MSG_SEND);
    if (status != FT_SUCCESS)
    {
        return ERR_IF;
    }

    /*
     * Allocate RX descriptors, 1 RxBD at a time.
     */
    for (i = 0; i < FXMAC_MSG_RX_PBUFS_LENGTH; i++)
    {
        if (instance_p->feature & FXMAC_MSG_OS_CONFIG_JUMBO)
        {
            p = pbuf_alloc(PBUF_RAW, FXMAC_MSG_MAX_FRAME_SIZE_JUMBO, PBUF_RAM);
        }
        else
        {
            p = pbuf_alloc(PBUF_RAW, FXMAC_MSG_MAX_FRAME_SIZE, PBUF_POOL);
        }

        if (!p)
        {
#if LINK_STATS
            lwip_stats.link.memerr++;
            lwip_stats.link.drop++;
#endif
            FXMAC_MSG_OS_PRINT_E("Unable to alloc pbuf in InitDma.");
            return ERR_IF;
        }
        status = FXmacMsgBdRingAlloc(rxringptr, 1, &rxbd);
        if (status != FT_SUCCESS)
        {
            FXMAC_MSG_OS_PRINT_E("InitDma: Error allocating RxBD.");
            pbuf_free(p);
            return ERR_IF;
        }
        /* Enqueue to HW */
        status = FXmacMsgBdRingToHw(rxringptr, 1, rxbd);
        if (status != FT_SUCCESS)
        {
            FXMAC_MSG_OS_PRINT_E("Error: committing RxBD to HW.");
            pbuf_free(p);
            FXmacMsgBdRingUnAlloc(rxringptr, 1, rxbd);
            return ERR_IF;
        }

        bdindex = FXMAC_MSG_BD_TO_INDEX(rxringptr, rxbd);
        temp = (u32 *)rxbd;
        *temp = 0;
        if (bdindex == (FXMAC_MSG_RX_PBUFS_LENGTH - 1))
        {
            *temp = 0x00000002;
        }
        temp++;
        *temp = 0;
        DSB();

        if (instance_p->feature & FXMAC_MSG_OS_CONFIG_JUMBO)
        {
            FCacheDCacheInvalidateRange((uintptr)p->payload, (uintptr)MAX_FRAME_SIZE_JUMBO);
        }
        else
        {
            FCacheDCacheInvalidateRange((uintptr)p->payload, (uintptr)FXMAC_MSG_MAX_FRAME_SIZE);
        }
        FXMAC_MSG_BD_SET_ADDRESS_RX(rxbd, (uintptr)p->payload);

        instance_p->buffer.rx_pbufs_storage[bdindex] = (uintptr)p;
    }

    FXmacMsgSetQueuePtr(&(instance_p->instance), instance_p->instance.tx_bd_queue.bdring.base_bd_addr, 0, (u16)FXMAC_MSG_SEND);
    FXmacMsgSetQueuePtr(&(instance_p->instance), instance_p->instance.rx_bd_queue.bdring.base_bd_addr, 0, (u16)FXMAC_MSG_RECV);

    return 0;
}

static void FreeOnlyTxPbufs(FXmacMsgOs *instance_p)
{
    u32 index;
    struct pbuf *p;

    for (index = 0; index < (FXMAC_MSG_TX_PBUFS_LENGTH); index++)
    {
        if (instance_p->buffer.tx_pbufs_storage[index] != 0)
        {
            p = (struct pbuf *)instance_p->buffer.tx_pbufs_storage[index];
            pbuf_free(p);
            instance_p->buffer.tx_pbufs_storage[index] = (uintptr)NULL;
        }
        instance_p->buffer.tx_pbufs_storage[index] = (uintptr)0;
    }
}


static void FreeOnlyRxPbufs(FXmacMsgOs *instance_p)
{
    u32 index;
    struct pbuf *p;

    for (index = 0; index < (FXMAC_MSG_RX_PBUFS_LENGTH); index++)
    {
        if (instance_p->buffer.rx_pbufs_storage[index] != 0)
        {
            p = (struct pbuf *)instance_p->buffer.rx_pbufs_storage[index];
            pbuf_free(p);
            instance_p->buffer.rx_pbufs_storage[index] = (uintptr)0;
        }
    }
}


static void FreeTxRxPbufs(FXmacMsgOs *instance_p)
{
    u32 rx_queue_len = 0;
    struct pbuf *p;
    /* first :free PqQueue data */
    rx_queue_len = FXmacPqQlength(&instance_p->recv_q);

    while (rx_queue_len)
    {
        /* return one packet from receive q */
        p = (struct pbuf *)FXmacPqDequeue(&instance_p->recv_q);
        pbuf_free(p);
        FXMAC_MSG_OS_PRINT_E("Delete queue %p", p);
        rx_queue_len--;
    }
    FreeOnlyTxPbufs(instance_p);
    FreeOnlyRxPbufs(instance_p);

}



static void ResetDma(FXmacMsgOs *instance_p)
{
    u8 txqueuenum;

    FXmacMsgBdRing *txringptr = &FXMAC_MSG_GET_TXRING(instance_p->instance);
    FXmacMsgBdRing *rxringptr = &FXMAC_MSG_GET_RXRING(instance_p->instance);

    FXmacMsgBdringPtrReset(txringptr, instance_p->buffer.tx_bdspace);
    FXmacMsgBdringPtrReset(rxringptr, instance_p->buffer.rx_bdspace);

    FXmacMsgSetQueuePtr(&(instance_p->instance), instance_p->instance.tx_bd_queue.bdring.base_bd_addr, 0, (u16)FXMAC_MSG_SEND);
    FXmacMsgSetQueuePtr(&(instance_p->instance), instance_p->instance.rx_bd_queue.bdring.base_bd_addr, 0, (u16)FXMAC_MSG_RECV);
}

void FXmacMsgLinkChange(void *args)
{
    u32 ctrl;
    u32 link, link_status;
    u32 speed;
    u32 speed_bit;
    u32 duplex;
    u32 status = FT_SUCCESS;

    FXmacMsgCtrl *xmac_p;
    FXmacMsgOs *instance_p;

    instance_p = (FXmacMsgOs *)args;
    xmac_p = &instance_p->instance;

    if (xmac_p->config.interface == FXMAC_MSG_PHY_INTERFACE_MODE_SGMII)
    {
        ctrl = FXMAC_MSG_READ(xmac_p, FXMAC_MSG_PCS_AN_LP);
        link = (ctrl & BIT(FXMAC_MSG_PCS_LINK_PARTNER_NEXT_PAGE_INDEX)) >> FXMAC_MSG_PCS_LINK_PARTNER_NEXT_PAGE_INDEX;

        switch (link)
        {
            case 0:
                FXMAC_MSG_OS_PRINT_I("Link status is down");
                link_status = FXMAC_MSG_LINKDOWN;
                break;
            case 1:
                FXMAC_MSG_OS_PRINT_I("Link status is up");
                link_status = FXMAC_MSG_LINKUP;
                break;
            default:
                FXMAC_MSG_OS_PRINT_E("Link status is error 0x%x ", link);
                return;
        }

        if (link_status == FXMAC_MSG_LINKUP)
        {
            if (link_status != xmac_p->link_status)
            {
                xmac_p->link_status = FXMAC_MSG_NEGOTIATING;
                FXMAC_MSG_OS_PRINT_I("Need NEGOTIATING.");
            }
        }
        else
        {
            xmac_p->link_status = FXMAC_MSG_LINKDOWN;
        }
    }
}

/* phy */

/**
 * @name: FXmacMsgPhyLinkDetect
 * @msg:  获取当前link status
 * @note:
 * @param {FXmac} *fxmac_p
 * @param {u32} phy_addr
 * @return {*} 1 is link up , 0 is link down
 */
static u32 FXmacMsgPhyLinkDetect(FXmacMsgCtrl *xmac_p, u32 phy_addr)
{
    u16 status;

    /* Read Phy Status register twice to get the confirmation of the current link status. */
    status = FXmacMsgMdioDataReadC22(xmac_p, phy_addr, PHY_STATUS_REG_OFFSET);

    if (status & PHY_STAT_LINK_STATUS)
    {
        return 1;
    }
    return 0;
}

static u32 FXmacPhyAutonegStatus(FXmacMsgCtrl *xmac_p, u32 phy_addr)
{
    u16 status;

    /* Read Phy Status register twice to get the confirmation of the current link status. */
    status = FXmacMsgMdioDataReadC22(xmac_p, phy_addr, PHY_STATUS_REG_OFFSET);

    if (status & PHY_STATUS_AUTONEGOTIATE_COMPLETE)
    {
        return 1;
    }
    return 0;
}

enum lwip_port_link_status FXmacMsgLwipPortLinkDetect(FXmacMsgOs *instance_p)
{
    u32 link_speed, phy_link_status;
    FXmacMsgCtrl *xmac_p = &instance_p->instance;

    if (xmac_p->is_ready != (u32)FT_COMPONENT_IS_READY)
    {
        return ETH_LINK_UNDEFINED;
    }

    phy_link_status = FXmacMsgPhyLinkDetect(xmac_p, xmac_p->phy_address);

    if ((xmac_p->link_status == FXMAC_MSG_LINKUP) && (!phy_link_status))
    {
        xmac_p->link_status = FXMAC_MSG_LINKDOWN;
    }

    switch (xmac_p->link_status)
    {
        case FXMAC_MSG_LINKUP:
            return ETH_LINK_UP;
        case FXMAC_MSG_LINKDOWN:
            xmac_p->link_status = FXMAC_MSG_NEGOTIATING;
            FXMAC_MSG_OS_PRINT_D("Ethernet Link down.");
            return ETH_LINK_DOWN;
        case FXMAC_MSG_NEGOTIATING:
            if ((phy_link_status == FXMAC_MSG_LINKUP) && FXmacPhyAutonegStatus(xmac_p, xmac_p->phy_address))
            {
                err_t phy_ret;
                phy_ret = FXmacMsgPhyInit(xmac_p, xmac_p->config.speed, xmac_p->config.duplex, xmac_p->config.auto_neg,XMAC_PHY_RESET_DISABLE);

                if (phy_ret != FT_SUCCESS)
                {
                    FXMAC_MSG_OS_PRINT_E("FXmacMsgPhyInit is error.");
                    return ETH_LINK_DOWN;
                }
                FXmacMsgInterfaceConfig(xmac_p, 0);

                /* Initiate Phy setup to get link speed */
                xmac_p->link_status = FXMAC_MSG_LINKUP;
                FXMAC_MSG_OS_PRINT_D("Ethernet Link up.");
                return ETH_LINK_UP;
            }
            return ETH_LINK_DOWN;
        default:
            return ETH_LINK_DOWN;
    }
}

enum lwip_port_link_status FXmacMsgPhyReconnect(struct LwipPort *xmac_netif_p)
{
    FXmacMsgCtrl *xmac_p;
    FXmacMsgOs *instance_p;

    FASSERT(xmac_netif_p != NULL);
    FASSERT(xmac_netif_p->state != NULL);

    instance_p = (FXmacMsgOs *)(xmac_netif_p->state);

    xmac_p = &instance_p->instance;

    if (xmac_p->config.interface == FXMAC_MSG_PHY_INTERFACE_MODE_SGMII)
    {
        InterruptMask(xmac_p->config.queue_irq_num[0]);
        if (xmac_p->link_status == FXMAC_MSG_NEGOTIATING)
        {
            /* 重新自协商 */
            err_t phy_ret;
            phy_ret = FXmacMsgPhyInit(xmac_p, xmac_p->config.speed, xmac_p->config.duplex, xmac_p->config.auto_neg, XMAC_PHY_RESET_DISABLE);
            if (phy_ret != FT_SUCCESS)
            {
                FXMAC_MSG_OS_PRINT_I("FXmacMsgPhyInit is error.");
                InterruptUmask(xmac_p->config.queue_irq_num[0]);
                return ETH_LINK_DOWN;
            }
            FXmacMsgInterfaceConfig(xmac_p, 0);
            xmac_p->link_status = FXMAC_MSG_LINKUP;
        }

        InterruptUmask(xmac_p->config.queue_irq_num[0]);

        switch (xmac_p->link_status)
        {
            case FXMAC_MSG_LINKDOWN:
                return ETH_LINK_DOWN;
            case FXMAC_MSG_LINKUP:
                return ETH_LINK_UP;
            default:
                return ETH_LINK_DOWN;
        }
    }
    else if ((xmac_p->config.interface == FXMAC_MSG_PHY_INTERFACE_MODE_RMII) || (xmac_p->config.interface == FXMAC_MSG_PHY_INTERFACE_MODE_RGMII))
    {
        return FXmacMsgLwipPortLinkDetect(instance_p);
    }
    else
    {
        switch (xmac_p->link_status)
        {
            case FXMAC_MSG_LINKDOWN:
                return ETH_LINK_DOWN;
            case FXMAC_MSG_LINKUP:
                return ETH_LINK_UP;
            default:
                return ETH_LINK_DOWN;
        }
    }
}

static void FxmacOsIntrHandler(s32 vector, void *args)
{
    isr_calling_flg++;
    FXmacMsgIntrHandler(vector, args);
    isr_calling_flg--;
}

static void FXmacDeinitIsr(FXmacMsgOs *instance_p)
{
    InterruptMask(instance_p->instance.config.queue_irq_num[0]);
}

static void FXmacMsgSetupIsr(FXmacMsgOs *instance_p)
{
    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(instance_p->instance.config.queue_irq_num[0], cpu_id);
    /* Setup callbacks */
    FXmacMsgSetHandler(&instance_p->instance, FXMAC_MSG_HANDLER_DMASEND, FXmacMsgSendHandler, instance_p);
    FXmacMsgSetHandler(&instance_p->instance, FXMAC_MSG_HANDLER_DMARECV, FXmacRecvSemaphoreHandler, instance_p);
    FXmacMsgSetHandler(&instance_p->instance, FXMAC_MSG_HANDLER_LINKCHANGE, FXmacMsgLinkChange, instance_p);
    InterruptSetPriority(instance_p->instance.config.queue_irq_num[0], XMAC_OS_IRQ_PRIORITY_VALUE);
    InterruptInstall(instance_p->instance.config.queue_irq_num[0], FxmacOsIntrHandler, &instance_p->instance, "fxmac_msg");
    InterruptUmask(instance_p->instance.config.queue_irq_num[0]);
}

/*  init fxmac instance */
static void FXmacMsgInitOnError(FXmacMsgOs *instance_p)
{
    FXmacMsgCtrl *xmac_p;
    u32 status = FT_SUCCESS;
    xmac_p = &instance_p->instance;

    /* set mac address */
    status = FXmacMsgSetMacAddress(xmac_p, (void *)(instance_p->hwaddr));
    if (status != FT_SUCCESS)
    {
        FXMAC_MSG_OS_PRINT_E("In %s:Emac Mac Address set failed...", __func__);
    }
}

/* 设置特性 */
void FXmacMsgFeatureSetOptions(u32 feature, FXmacMsgCtrl* xmac_p)
{
    u16 cmd_id, cmd_subid;
    cmd_id = FXMAC_MSG_CMD_SET;

    /* 巨帧 */
    if (feature & FXMAC_MSG_OS_CONFIG_JUMBO)
    {
		cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_JUMBO;
	    FXmacMsgSendMessage(xmac_p, cmd_id, cmd_subid, NULL, 0, 0);
        FXMAC_MSG_OS_PRINT_I("FXMAC_MSG_JUMBO_ENABLE_OPTION is ok");
    }
    
    /* 单播 */
    if (feature & FXMAC_MSG_OS_CONFIG_UNICAST_ADDRESS_FILITER)
    {
        FXMAC_MSG_OS_PRINT_I("FXMAC_MSG_UNICAST_OPTION is ok");
    }

    /* 多播 */
    if (feature & FXMAC_MSG_OS_CONFIG_MULTICAST_ADDRESS_FILITER)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_MC;
        FXmacMsgSendMessage(xmac_p, cmd_id, cmd_subid, NULL, 0, 1);
        FXMAC_MSG_OS_PRINT_I("FXMAC_MSG_MULTICAST_OPTION is ok");
    }
    
    /* 全帧 */
    if (feature & FXMAC_MSG_OS_CONFIG_COPY_ALL_FRAMES)
    {
		cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_PROMISE;
		FXmacMsgSendMessage(xmac_p, cmd_id, cmd_subid, NULL, 0, 0);
        FXMAC_MSG_OS_PRINT_I("FXMAC_MSG_PROMISC_OPTION is ok");
    }
    
    /* 关闭FCS(帧校验序列)校验 */
    if (feature & FXMAC_MSG_OS_CONFIG_CLOSE_FCS_CHECK)
    {
        FXMAC_MSG_OS_PRINT_I("FXMAC_MSG_FCS_STRIP_OPTION is ok");
    }
}

/* step 1: initialize instance */
/* step 2: depend on config set some options : JUMBO / IGMP */
/* step 3: FXmacSelectClk */
/* step 4: FXmacInitInterface */
/* step 5: initialize phy */
/* step 6: initialize dma */
/* step 7: initialize interrupt */
/* step 8: start mac */
FError FXmacMsgOsInit(FXmacMsgOs *instance_p)
{
    FXmacMsgConfig mac_config;
    const FXmacMsgConfig *mac_config_p;
    FXmacMsgPhyInterface interface = FXMAC_MSG_PHY_INTERFACE_MODE_SGMII;
    FXmacMsgCtrl *xmac_p;
    FError status;
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->mac_config.instance_id < FXMAC_MSG_NUM);

    xmac_p = &instance_p->instance;
    FXMAC_MSG_OS_PRINT_I("instance_id IS %d", instance_p->mac_config.instance_id);

    /* 获取默认配置 */
    mac_config_p = FXmacMsgLookupConfig(instance_p->mac_config.instance_id);
    if (mac_config_p == NULL)
    {
        FXMAC_MSG_OS_PRINT_E("FXmacMsgLookupConfig is error , instance_id is %d", instance_p->mac_config.instance_id);
        return FREERTOS_XMAC_MSG_INIT_ERROR;
    }
    mac_config = *mac_config_p;

    /* 用户配置 */
    /* PHY接口 */
    switch (instance_p->mac_config.interface)
    {
        case FXMAC_MSG_OS_INTERFACE_SGMII:
            interface = FXMAC_MSG_PHY_INTERFACE_MODE_SGMII;
            FXMAC_MSG_OS_PRINT_I("SGMII select.");
            break;
        case FXMAC_MSG_OS_INTERFACE_RMII:
            interface = FXMAC_MSG_PHY_INTERFACE_MODE_RMII;
            FXMAC_MSG_OS_PRINT_I("RMII select.");
            break;
        case FXMAC_MSG_OS_INTERFACE_RGMII:
            FXMAC_MSG_OS_PRINT_I("RGMII select.");
            interface = FXMAC_MSG_PHY_INTERFACE_MODE_RGMII;
            break;
        default:
            FXMAC_MSG_OS_PRINT_E("Update interface is error , interface is %d", instance_p->mac_config.instance_id);
            return FREERTOS_XMAC_MSG_INIT_ERROR;
    }
    mac_config.interface = interface;
    /* 自协商 */
    if (instance_p->mac_config.autonegotiation)
    {
        mac_config.auto_neg = 1;
    }
    else
    {
        mac_config.auto_neg = 0;
    }
    /* 速度 */
    switch (instance_p->mac_config.phy_speed)
    {
        case FXMAC_MSG_PHY_SPEED_10M:
            mac_config.speed = FXMAC_MSG_SPEED_10;
            break;
        case FXMAC_MSG_PHY_SPEED_100M:
            mac_config.speed = FXMAC_MSG_SPEED_100;
            break;
        case FXMAC_MSG_PHY_SPEED_1000M:
            mac_config.speed = FXMAC_MSG_SPEED_1000;
            break;
        default:
            FXMAC_MSG_OS_PRINT_E("Setting speed is not valid , speed is %d", instance_p->mac_config.phy_speed);
            return FREERTOS_XMAC_MSG_INIT_ERROR;
    }
    /* 双工模式 */
    switch (instance_p->mac_config.phy_duplex)
    {
        case FXMAC_MSG_PHY_HALF_DUPLEX:
            mac_config.duplex = 0;
            break;
        case FXMAC_MSG_PHY_FULL_DUPLEX:
            mac_config.duplex = 1;
            break;
    }

    /* 配置初始化 */
    status = FXmacMsgCfgInitialize(xmac_p, &mac_config);
    if (status != FT_SUCCESS)
    {
        FXMAC_MSG_OS_PRINT_E("In %s:Xmac Msg Configuration Failed....", __func__);
    }

    /* 初始化MSG消息队列 */
    FXmacMsgInitRing(xmac_p);

    /* 获取配置参数 */
    FXmacMsgGetFeatureAll(xmac_p);

    /* 硬件重置 */
    FXmacMsgResetHw(xmac_p);    

    /* 设置特性 */
    FXmacMsgFeatureSetOptions(instance_p->feature, xmac_p);

    /* 初始化硬件 */
    FXmacMsgInitHw(xmac_p, instance_p->hwaddr);

    /* 初始化PHY */
    if (xmac_p->config.interface != FXMAC_MSG_PHY_INTERFACE_MODE_USXGMII)
    {
        status = FXmacMsgPhyInit(xmac_p, xmac_p->config.speed, xmac_p->config.duplex, xmac_p->config.auto_neg, FXMAC_MSG_PHY_RESET_ENABLE);
        if (status != FT_SUCCESS)
        {
            FXMAC_MSG_OS_PRINT_W("FXmacMsgPhyInit is error \r\n");
        }
    }

    /* 初始化PHY接口 */
    FXmacMsgInterfaceConfig(xmac_p, 0);

    /* 初始化DMA描述符 */
    FXmacMsgInitDma(instance_p);

    /* 初始化中断 */
    FXmacMsgSetupIsr(instance_p);

    return FT_SUCCESS;
}


/**
 * @name: FXmacMsgOsRx
 * @msg: moving packets into lwip stack
 * @note: 
 * @param {FXmacMsgOs} *instance_p
 * @param {void} *pbuf
 * @return {*}
 */
void FXmacMsgOsRx(FXmacMsgOs *instance_p, void *pbuf)
{
    FASSERT(instance_p != NULL);
    FASSERT(pbuf != NULL);

    struct netif *netif;
    struct pbuf *p;
    struct eth_hdr *ethhdr;

    netif =  (struct netif *)instance_p->netif;
    p = (struct pbuf *)pbuf;
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

    return;
}

static FError FXmacMsgOsOutput(FXmacMsgOs *instance_p, struct pbuf *p)
{
    FError status = 0;
    status = FXmacMsgSgsend(instance_p, p);
    if (status != FT_SUCCESS)
    {
#if LINK_STATS
        lwip_stats.link.drop++;
#endif
    }

#if LINK_STATS
    lwip_stats.link.xmit++;
#endif /* LINK_STATS */

    return status;
}

FError FXmacMsgOsTx(FXmacMsgOs *instance_p, void *pbuf)
{
    FASSERT(instance_p != NULL);
    FASSERT(pbuf != NULL);

    FXmacMsgBdRing *txring;
    struct pbuf *p;
    FError ret = FT_SUCCESS;

    p = (struct pbuf *)pbuf;
    sys_prot_t lev;
    lev = sys_arch_protect();

    txring = &(FXMAC_MSG_GET_TXRING(instance_p->instance));
    FXmacMsgProcessSentBds(instance_p, txring, FXMAC_MSG_TX_PBUFS_LENGTH);  

    if (IsTxSpaceAvailable(instance_p))
    {
        ret = FXmacMsgOsOutput(instance_p, p);
    }
    else
    {
#if LINK_STATS
        lwip_stats.link.drop++;
#endif
        FXMAC_MSG_OS_PRINT_E("Pack dropped, no space.");
        ret = FREERTOS_XMAC_MSG_NO_VALID_SPACE;
    }

    sys_arch_unprotect(lev);
    return ret;
}

FXmacMsgOs *FXmacMsgOsGetInstancePointer(FXmacMsgPhyControl *config_p)
{
    FXmacMsgOs *instance_p;
    FASSERT(config_p != NULL);
    FASSERT(config_p->instance_id < FXMAC_MSG_NUM);
    FASSERT_MSG(config_p->interface < FXMAC_MSG_OS_INTERFACE_LENGTH, "config_p->interface %d is over %d", config_p->interface, FXMAC_MSG_OS_INTERFACE_LENGTH);
    FASSERT_MSG(config_p->autonegotiation <= 1, "config_p->autonegotiation %d is over 1", config_p->autonegotiation);
    FASSERT_MSG(config_p->phy_speed <= FXMAC_MSG_PHY_SPEED_1000M, "config_p->phy_speed %d is over 1000", config_p->phy_speed);
    FASSERT_MSG(config_p->phy_duplex <= FXMAC_MSG_PHY_FULL_DUPLEX, "config_p->phy_duplex %d is over FXMAC_MSG_PHY_FULL_DUPLEX", config_p->phy_duplex);

    instance_p = &fxmac_os_instace[config_p->instance_id];
    memcpy(&instance_p->mac_config, config_p, sizeof(FXmacMsgPhyControl));
    return instance_p;
}


void FXmacMsgOsStop(FXmacMsgOs *instance_p)
{
    FASSERT(instance_p != NULL);
    /* step 1 close interrupt  */
    FXmacDeinitIsr(instance_p);
    /* step 2 close mac controler  */
    FXmacMsgStop(&instance_p->instance);
    /* step 3 free all pbuf */
    FreeTxRxPbufs(instance_p);
}

void FXmacMsgOsStart(FXmacMsgOs *instance_p)
{
    FASSERT(instance_p != NULL);
    /* start mac */
    FXmacMsgStart(&instance_p->instance);
}
