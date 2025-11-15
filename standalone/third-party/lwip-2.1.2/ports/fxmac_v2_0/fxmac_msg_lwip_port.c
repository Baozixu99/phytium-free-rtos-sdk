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
 * FilePath: fxmac_msg_lwip_port.c
 * Date: 2025-01-22 14:59:22
 * LastEditTime: 2025-01-22 14:59:22
 * Description:  This file is xmac msg portable code for lwip port input,output,status check.
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huangjin     2025/01/22            first release
 */
#include "fparameters.h"
#include "fassert.h"
#include "fcache.h"
#include "fcpu_info.h"
#include "sys_arch.h"
#include "faarch.h"
#include "finterrupt.h"
#include "fdebug.h"

#include "fxmac_msg_lwip_port.h"
#include "lwip_port.h"

#include "fxmac_msg.h"
#include "fxmac_msg_bd.h"
#include "fxmac_msg_bdring.h"
#include "fxmac_msg_phy.h"
#include "fxmac_msg_hw.h"
#include "fxmac_msg_common.h"
#include "fdrivers_port.h"
#include "eth_ieee_reg.h"

#define FXMAC_MSG_LWIP_PORT_XMAC_DEBUG_TAG "FXMAC_MSG_LWIP_PORT_XMAC"
#define FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E(format, ...) \
    FT_DEBUG_PRINT_E(FXMAC_MSG_LWIP_PORT_XMAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I(format, ...) \
    FT_DEBUG_PRINT_I(FXMAC_MSG_LWIP_PORT_XMAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_LWIP_PORT_XMAC_PRINT_D(format, ...) \
    FT_DEBUG_PRINT_D(FXMAC_MSG_LWIP_PORT_XMAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_LWIP_PORT_XMAC_PRINT_W(format, ...) \
    FT_DEBUG_PRINT_W(FXMAC_MSG_LWIP_PORT_XMAC_DEBUG_TAG, format, ##__VA_ARGS__)

#define FXMAC_MSG_BD_TO_INDEX(ringptr, bdptr) \
    (((uintptr)bdptr - (uintptr)(ringptr)->base_bd_addr) / (ringptr)->separation)

// static void FXmacMsgInitOnError(FXmacMsgLwipPort *instance_p);
// static void FXmacMsgSetupIsr(FXmacMsgLwipPort *instance_p);

static FXmacMsgLwipPort fxmac_msg_lwip_port_instance[FXMAC_MSG_NUM] = {
    [FXMAC0_MSG_ID] = 0,
#if defined(FXMAC1_MSG_ID)
    [FXMAC1_MSG_ID] = {{{0}}},
#endif
#if defined(FXMAC2_MSG_ID)
    [FXMAC2_MSG_ID] = {{{0}}},
#endif
#if defined(FXMAC3_MSG_ID)
    [FXMAC3_MSG_ID] = {{{0}}},
#endif
};


/* queue */

void FXmacMsgQueueInit(PqQueue *q)
{
    FASSERT(q != NULL);
    q->head = q->tail = q->len = 0;
}

int FXmacMsgPqEnqueue(PqQueue *q, void *p)
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

void *FXmacMsgPqDequeue(PqQueue *q)
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

int FXmacMsgPqQlength(PqQueue *q)
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
static u32 IsTxSpaceAvailable(FXmacMsgLwipPort *instance_p)
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
 */
void FXmacMsgProcessSentBds(FXmacMsgLwipPort *instance_p, FXmacMsgBdRing *txring)
{
    FXmacMsgBd *txbdset;
    FXmacMsgBd *curbdpntr;
    u32 n_bds;
    FError status;
    u32 n_pbufs_freed = 0;
    u32 bdindex = 0;
    struct pbuf *p;
    u32 *temp;

    while (1)
    {
        /* obtain processed BD's */
        n_bds = FXmacMsgBdRingFromHwTx(txring, FXMAX_TX_PBUFS_LENGTH, &txbdset);
        if (n_bds == 0)
        {
            return;
        }
        else
        {
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_D("n_bds = %d\r\n", n_bds);
        }

        /* 清除描述符，释放pubf */
        /* free the processed BD's */
        n_pbufs_freed = n_bds;
        curbdpntr = txbdset;
        while (n_pbufs_freed > 0)
        {
            bdindex = FXMAC_MSG_BD_TO_INDEX(txring, curbdpntr);
            temp = (u32 *)curbdpntr;
            *temp = 0; /* Word 0 */
            temp++;

            if (bdindex == (FXMAX_TX_PBUFS_LENGTH - 1))
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
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("Failure while freeing in Tx Done "
                                             "ISR\r\n");
        }
    }
    return;
}

void FXmacMsgSendHandler(void *arg)
{
    FXmacMsgLwipPort *instance_p;
    FXmacMsgBdRing *txringptr;

    instance_p = (FXmacMsgLwipPort *)arg;
    txringptr = &(FXMAC_MSG_GET_TXRING(instance_p->instance));

    /* If Transmit done interrupt is asserted, process completed BD's */
    FXmacMsgProcessSentBds(instance_p, txringptr);
}

FError FXmacMsgSgsend(FXmacMsgLwipPort *instance_p, struct pbuf *p)
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
    /* 计算pubf的数量 */
    for (q = p, n_pbufs = 0; q != NULL; q = q->next)
    {
        n_pbufs++;
    }

    /* obtain as many BD's */
    /* 在描述符环形队列中申请对应数量的描述符 */
    status = FXmacMsgBdRingAlloc(txring, n_pbufs, &txbdset);
    if (status != FT_SUCCESS)
    {
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("sgsend: Error allocating TxBD\r\n");
        return ERR_GENERAL;
    }

    /* 设置每个描述符的Word0、Word1 */
    for (q = p, txbd = txbdset; q != NULL; q = q->next)
    {
        bdindex = FXMAC_MSG_BD_TO_INDEX(txring, txbd);

        if (instance_p->buffer.tx_pbufs_storage[bdindex])
        {
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("PBUFS not available\r\n");
            goto err;
        }

        /* Send the data from the pbuf to the interface, one pbuf at a
           time. The size of the data in each pbuf is kept in the ->len
           variable. */
        FLwipDCacheRangeFlush((uintptr)q->payload, (uintptr)q->len);
        FXMAC_MSG_BD_SET_ADDRESS_TX(txbd, (uintptr)q->payload);

        if (instance_p->feature & FXMAC_MSG_LWIP_PORT_CONFIG_JUMBO)
        {
            max_fr_size = FXMAC_MSG_MAX_FRAME_SIZE_JUMBO;
        }
        else
        {
            max_fr_size = FXMAC_MSG_MAX_FRAME_SIZE;
        }

        if (q->len > max_fr_size)
        {
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_D("max_fr_size = %d", max_fr_size);
            FXMAC_MSG_BD_SET_LENGTH(txbd, max_fr_size & 0x3FFF);
        }
        else
        {
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_D("q->len = %d", q->len);
            FXMAC_MSG_BD_SET_LENGTH(txbd, q->len & 0x3FFF);
        }

        instance_p->buffer.tx_pbufs_storage[bdindex] = (uintptr)q;

        pbuf_ref(q);
        last_txbd = txbd;
        FXMAC_MSG_BD_CLEAR_LAST(txbd);
        txbd = FXMAC_MSG_BD_RING_NEXT(txring, txbd);
    }
    FXMAC_MSG_BD_SET_LAST(last_txbd);
    /* 执行到这描述符已经全部设置完毕 */

    /* 检查描述符 */
    FXMAC_MSG_LWIP_PORT_XMAC_PRINT_D("last_txbd word0 = 0x%x, word1 = 0x%x",
                                     (*last_txbd)[0], (*last_txbd)[1]);

    /* The bdindex always points to the first free_head in tx_bdrings */
    /* 获取最后可用描述符的下一位编号 */
    bdindex = FXMAC_MSG_BD_TO_INDEX(txring, txbd);

    /* For fragmented packets, remember the 1st BD allocated for the 1st
       packet fragment. The used bit for this BD should be cleared at the end
       after clearing out used bits for other fragments. For packets without
       just remember the allocated BD. */
    /* 多pubf帧对应多描述符，软件清除USED位，以便继续使用描述符
    FXMAC_MSG_TXBUF_USED_MASK
    0：还未被硬件使用
    1：已被硬件使用，数据已成功发送传输出去 */
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
    FXMAC_MSG_LWIP_PORT_XMAC_PRINT_D("Update USED temp_txbd word0 = 0x%x, word1 = 0x%x",
                                     (*temp_txbd)[0], (*temp_txbd)[1]);

    /* 将设置好的描述符提交给硬件处理 */
    status = FXmacMsgBdRingToHw(txring, n_pbufs, txbdset);
    if (status != FT_SUCCESS)
    {
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("sgsend: Error submitting TxBD\r\n");
        goto err;
    }

    /* 通过TX尾帧进行发送 */
    /*
    1、初始化阶段：使能TX尾指针功能
    2、尾指针的配置：尾指针指向当前报文所用描述符的后一个，既USED描述符位置
    */
    DSB();
    FXMAC_MSG_WRITE((&instance_p->instance), FXMAC_MSG_TX_PTR(0), bdindex);

    return FT_SUCCESS;
err:
    FXmacMsgBdRingUnAlloc(txring, n_pbufs, txbdset);
    return status;
}

void SetupRxBds(FXmacMsgLwipPort *instance_p, FXmacMsgBdRing *rxring)
{
    FXmacMsgBd *rxbd;
    FError status;
    struct pbuf *p;
    u32 freebds;
    u32 bdindex = 0;
    u32 *temp;
    freebds = FXMAC_MSG_BD_RING_GET_FREE_CNT(rxring);
    while (freebds > 0)
    {
        freebds--;

        if (instance_p->feature & FXMAC_MSG_LWIP_PORT_CONFIG_JUMBO)
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
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("unable to alloc pbuf in "
                                             "recv_handler\r\n");
            return;
        }
        status = FXmacMsgBdRingAlloc(rxring, 1, &rxbd);
        if (status != FT_SUCCESS)
        {
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("SetupRxBds: Error allocating RxBD\r\n");
            pbuf_free(p);
            return;
        }
        status = FXmacMsgBdRingToHw(rxring, 1, rxbd);
        if (status != FT_SUCCESS)
        {
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("Error committing RxBD to hardware: ");
            if (status == FXMAC_MSG_ERR_SG_LIST)
            {
                FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("XST_DMA_SG_LIST_ERROR: this function "
                                                 "was called out of sequence with "
                                                 "FXmacMsgBdRingAlloc()\r\n");
            }
            else
            {
                FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("set of BDs was rejected because the "
                                                 "first BD did not have its "
                                                 "start-of-packet bit set, or the last "
                                                 "BD did not have its end-of-packet "
                                                 "bit set, or any one of the BD set "
                                                 "has 0 as length value\r\n");
            }

            pbuf_free(p);
            FXmacMsgBdRingUnAlloc(rxring, 1, rxbd);
            return;
        }

        if (instance_p->feature & FXMAC_MSG_LWIP_PORT_CONFIG_JUMBO)
        {
            FLwipDCacheRangeInvalidate((uintptr)p->payload, (uintptr)FXMAC_MSG_MAX_FRAME_SIZE_JUMBO);
        }
        else
        {
            FLwipDCacheRangeInvalidate((uintptr)p->payload, (uintptr)FXMAC_MSG_MAX_FRAME_SIZE);
        }

        bdindex = FXMAC_MSG_BD_TO_INDEX(rxring, rxbd);
        temp = (u32 *)rxbd;
        if (bdindex == (FXMAX_RX_PBUFS_LENGTH - 1))
        {
            *temp = 0x00000002; /* Mask last descriptor in receive buffer list */
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

void FXmacMsgRecvIsrHandler(void *arg)
{
    FASSERT(arg != NULL);
    FXMAC_MSG_LWIP_PORT_XMAC_PRINT_D("FXmacMsgRecvIsrHandler!!!!!!!!!!!!!!");
    FXmacMsgLwipPort *instance_p = (FXmacMsgLwipPort *)arg;
    FXmacMsgCtrl *xmac_p = &instance_p->instance;
    /* 禁用接收完成中断 */
    FXmacMsgDisableIrq(xmac_p, 0, FXMAC_MSG_INT_RX_COMPLETE);
    instance_p->recv_flg++;
}

/* 接收处理函数 */
void FXmacMsgRecvHandler(void *arg)
{
    struct pbuf *p;
    FXmacMsgBd *rxbdset, *curbdptr;
    FXmacMsgBd *rxtailbdptr = NULL;
    FXmacMsgBdRing *rxring;
    volatile u32 bd_processed;
    u32 rx_bytes, k;
    u32 bdindex = 0;
    u32 rx_tail_bd_index = 0;
    u32 hash_match;
    FXmacMsgLwipPort *instance_p;
    FASSERT(arg != NULL);

    instance_p = (FXmacMsgLwipPort *)arg;
    rxring = &FXMAC_MSG_GET_RXRING(instance_p->instance);

    while (1)
    {
        bd_processed = FXmacMsgBdRingFromHwRx(rxring, FXMAX_RX_PBUFS_LENGTH, &rxbdset);
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
            if (instance_p->feature & FXMAC_MSG_LWIP_PORT_CONFIG_JUMBO)
            {
                rx_bytes = FXMAC_MSG_GET_RX_FRAME_SIZE(curbdptr);
            }
            else
            {
                rx_bytes = FXMAC_MSG_BD_GET_LENGTH(curbdptr);
            }
            pbuf_realloc(p, rx_bytes);
            /*  
            The value of hash_match indicates the hash result of the received packet 
               0: No hash match 
               1: Unicast hash match 
               2: Multicast hash match
               3: Reserved, the value is not legal
           */
            hash_match = FXMAC_MSG_BD_GET_HASH_MATCH(curbdptr);
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_D("hash_match is %d\r\n", hash_match);

            /* Invalidate RX frame before queuing to handle
             * L1 cache prefetch conditions on any architecture.
             */
            FLwipDCacheRangeInvalidate((uintptr)p->payload, rx_bytes);

            /* store it in the receive queue,
             * where it'll be processed by a different handler
             */
            if (FXmacMsgPqEnqueue(&instance_p->recv_q, (void *)p) < 0)
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
    }

    /* RX尾指针更新，告知硬件可用的描述符范围 */
    /*
    1、初始化阶段：使能RX尾指针功能
    2、尾指针的配置：尾指针指向当前报文所用描述符的后一个，既USED描述符位置
    */
    rx_tail_bd_index = FXMAC_MSG_BD_TO_INDEX(rxring, rxtailbdptr);
    DSB();
    FXMAC_MSG_WRITE((&instance_p->instance), FXMAC_MSG_RX_PTR(0), rx_tail_bd_index);

    return;
}

// void CleanDmaTxdescs(FXmacMsgLwipPort *instance_p)
// {
//     FXmacMsgBd bdtemplate;
//     FXmacMsgBdRing *txringptr;

//     txringptr = &FXMAC_GET_TXRING((instance_p->instance));
//     FXMAC_BD_CLEAR(&bdtemplate);
//     FXMAC_BD_SET_STATUS(&bdtemplate, FXMAC_TXBUF_USED_MASK);

//     FXmacMsgBdRingCreate(txringptr, (uintptr)instance_p->buffer.tx_bdspace,
//                       (uintptr)instance_p->buffer.tx_bdspace, BD_ALIGNMENT,
//                       sizeof(instance_p->buffer.tx_bdspace));

//     FXmacMsgBdRingClone(txringptr, &bdtemplate, FXMAC_SEND);
// }

FError FXmacMsgInitDma(FXmacMsgLwipPort *instance_p)
{
    FXmacMsgBd bdtemplate;
    FXmacMsgBdRing *rxringptr, *txringptr;
    FXmacMsgBd *rxbd;
    struct pbuf *p;
    FError status;
    int i;
    u32 bdindex = 0;
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
    FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("rxringptr: 0x%08x\r\n", rxringptr);
    FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("txringptr: 0x%08x\r\n", txringptr);

    FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("rx_bdspace: %p \r\n", instance_p->buffer.rx_bdspace);
    FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("tx_bdspace: %p \r\n", instance_p->buffer.tx_bdspace);

    /* Setup RxBD space. */
    FXMAC_MSG_BD_CLEAR(&bdtemplate);

    /* Create the RxBD ring */
    status = FXmacMsgBdRingCreate(rxringptr, (uintptr)instance_p->buffer.rx_bdspace,
                                  (uintptr)instance_p->buffer.rx_bdspace, BD_ALIGNMENT,
                                  FXMAX_RX_PBUFS_LENGTH);

    if (status != FT_SUCCESS)
    {
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("Error setting up RxBD space\r\n");
        return ERR_IF;
    }

    status = FXmacMsgBdRingClone(rxringptr, &bdtemplate, FXMAC_MSG_RECV);
    if (status != FT_SUCCESS)
    {
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("Error initializing RxBD space\r\n");
        return ERR_IF;
    }

    FXMAC_MSG_BD_CLEAR(&bdtemplate);
    FXMAC_MSG_BD_SET_STATUS(&bdtemplate, FXMAC_MSG_TXBUF_USED_MASK);

    /* Create the TxBD ring */
    status = FXmacMsgBdRingCreate(txringptr, (uintptr)instance_p->buffer.tx_bdspace,
                                  (uintptr)instance_p->buffer.tx_bdspace, BD_ALIGNMENT,
                                  FXMAX_TX_PBUFS_LENGTH);

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
    FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("Allocate RX descriptors, 1 RxBD at a time.");
    for (i = 0; i < FXMAX_RX_PBUFS_LENGTH; i++)
    {
        if (instance_p->feature & FXMAC_MSG_LWIP_PORT_CONFIG_JUMBO)
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
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("unable to alloc pbuf in InitDma\r\n");
            return ERR_IF;
        }
        status = FXmacMsgBdRingAlloc(rxringptr, 1, &rxbd);
        if (status != FT_SUCCESS)
        {
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("InitDma: Error allocating RxBD\r\n");
            pbuf_free(p);
            return ERR_IF;
        }
        /* Enqueue to HW */
        status = FXmacMsgBdRingToHw(rxringptr, 1, rxbd);
        if (status != FT_SUCCESS)
        {
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("Error: committing RxBD to HW\r\n");
            pbuf_free(p);
            FXmacMsgBdRingUnAlloc(rxringptr, 1, rxbd);
            return ERR_IF;
        }

        bdindex = FXMAC_MSG_BD_TO_INDEX(rxringptr, rxbd);
        temp = (u32 *)rxbd;
        *temp = 0;
        if (bdindex == (FXMAX_RX_PBUFS_LENGTH - 1))
        {
            *temp = 0x00000002; /* Marks last descriptor in receive buffer descriptor list */
        }
        temp++;
        *temp = 0; /* Clear word 1 in  descriptor */
        DSB();

        if (instance_p->feature & FXMAC_MSG_LWIP_PORT_CONFIG_JUMBO)
        {
            FLwipDCacheRangeInvalidate((uintptr)p->payload, (uintptr)FXMAC_MSG_MAX_FRAME_SIZE_JUMBO);
        }
        else
        {
            FLwipDCacheRangeInvalidate((uintptr)p->payload, (uintptr)FXMAC_MSG_MAX_FRAME_SIZE);
        }
        FXMAC_MSG_BD_SET_ADDRESS_RX(rxbd, (uintptr)p->payload);

        instance_p->buffer.rx_pbufs_storage[bdindex] = (uintptr)p;
    }

    FXmacMsgSetQueuePtr(&(instance_p->instance), instance_p->instance.tx_bd_queue.bdring.phys_base_addr,
                        0, (u16)FXMAC_MSG_SEND);
    FXmacMsgSetQueuePtr(&(instance_p->instance), instance_p->instance.rx_bd_queue.bdring.phys_base_addr,
                        0, (u16)FXMAC_MSG_RECV);

    return 0;
}

static void FreeOnlyTxPbufs(FXmacMsgLwipPort *instance_p)
{
    u32 index;
    struct pbuf *p;

    for (index = 0; index < (FXMAX_TX_PBUFS_LENGTH); index++)
    {
        if (instance_p->buffer.tx_pbufs_storage[index] != 0)
        {
            p = (struct pbuf *)instance_p->buffer.tx_pbufs_storage[index];
            pbuf_free(p);
            instance_p->buffer.tx_pbufs_storage[index] = (uintptr)NULL;
        }
    }
}

static void FreeOnlyRxPbufs(FXmacMsgLwipPort *instance_p)
{
    u32 index;
    struct pbuf *p;

    for (index = 0; index < (FXMAX_RX_PBUFS_LENGTH); index++)
    {
        if (instance_p->buffer.rx_pbufs_storage[index] != 0)
        {
            p = (struct pbuf *)instance_p->buffer.rx_pbufs_storage[index];
            pbuf_free(p);
            instance_p->buffer.rx_pbufs_storage[index] = (uintptr)NULL;
        }
    }
}


static void FreeTxRxPbufs(FXmacMsgLwipPort *instance_p)
{
    u32 rx_queue_len = 0;
    struct pbuf *p;
    /* first :free PqQueue data */

    rx_queue_len = FXmacMsgPqQlength(&instance_p->recv_q);

    while (rx_queue_len)
    {
        /* return one packet from receive q */
        p = (struct pbuf *)FXmacMsgPqDequeue(&instance_p->recv_q);
        pbuf_free(p);
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_W("delete queue %p", p);
        rx_queue_len--;
    }

    FreeOnlyTxPbufs(instance_p);
    FreeOnlyRxPbufs(instance_p);
}


/* Reset Tx and Rx DMA pointers after FXmacStop */
void FXmacMsgResetDma(FXmacMsgLwipPort *instance_p)
{
    FXmacMsgBdRing *txringptr = &FXMAC_MSG_GET_TXRING(instance_p->instance);
    FXmacMsgBdRing *rxringptr = &FXMAC_MSG_GET_RXRING(instance_p->instance);

    FXmacMsgBdringPtrReset(txringptr, instance_p->buffer.tx_bdspace);
    FXmacMsgBdringPtrReset(rxringptr, instance_p->buffer.rx_bdspace);

    FXmacMsgSetQueuePtr(&(instance_p->instance), instance_p->instance.tx_bd_queue.bdring.phys_base_addr,
                        0, (u16)FXMAC_MSG_SEND);
    FXmacMsgSetQueuePtr(&(instance_p->instance), instance_p->instance.rx_bd_queue.bdring.phys_base_addr,
                        0, (u16)FXMAC_MSG_RECV);
}

/* interrupt */
// static void FXmacHandleDmaTxError(FXmacMsgLwipPort *instance_p)
// {
//     s32_t status = FT_SUCCESS;
//     u32 dmacrreg;

//     FreeTxRxPbufs(instance_p);
//     status = FXmacCfgInitialize(&instance_p->instance, &instance_p->instance.config);

//     if (status != FT_SUCCESS)
//     {
//         FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("In %s:EmacPs Configuration Failed....\r\n", __func__);
//     }

//     /* initialize the mac */
//     // FXmacMsgInitOnError(instance_p); /* need to set mac filter address */
//     dmacrreg = FXMAC_READREG32(instance_p->instance.config.base_address, FXMAC_DMACR_OFFSET);
//     dmacrreg = dmacrreg | (FXMAC_DMACR_ORCE_DISCARD_ON_ERR_MASK); /* force_discard_on_err */
//     FXMAC_WRITEREG32(instance_p->instance.config.base_address, FXMAC_DMACR_OFFSET, dmacrreg);
//     // FXmacMsgSetupIsr(instance_p);
//     FXmacInitDma(instance_p);

//     FXmacStart(&instance_p->instance);
// }

// void FXmacHandleTxErrors(FXmacMsgLwipPort *instance_p)
// {
//     u32 netctrlreg;

//     netctrlreg = FXMAC_READREG32(instance_p->instance.config.base_address,
//                                  FXMAC_NWCTRL_OFFSET);
//     netctrlreg = netctrlreg & (~FXMAC_NWCTRL_TXEN_MASK);
//     FXMAC_WRITEREG32(instance_p->instance.config.base_address,
//                      FXMAC_NWCTRL_OFFSET, netctrlreg);
//     FreeOnlyTxPbufs(instance_p);

//     CleanDmaTxdescs(instance_p);
//     netctrlreg = FXMAC_READREG32(instance_p->instance.config.base_address, FXMAC_NWCTRL_OFFSET);
//     netctrlreg = netctrlreg | (FXMAC_NWCTRL_TXEN_MASK);
//     FXMAC_WRITEREG32(instance_p->instance.config.base_address, FXMAC_NWCTRL_OFFSET, netctrlreg);
// }

// void FXmacMsgErrorHandler(void *arg, u8 direction, u32 error_word)
// {
//     FXmacMsgBdRing *txring;
//     FXmacMsgLwipPort *instance_p;

//     instance_p = (FXmacMsgLwipPort *)(arg);
//     txring = &FXMAC_MSG_GET_TXRING((instance_p->instance));

//     /* MSG模式下没有发送状态寄存器(0x14)、接收状态寄存器(0x20) */
//     if (error_word != 0)
//     {
//         switch (direction)
//         {
//         case FXMAC_MSG_RECV:
//             if (error_word & FXMAC_RXSR_HRESPNOK_MASK)
//             {
//                 FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("Receive DMA error\r\n");
//                 FXmacHandleDmaTxError(instance_p);
//             }
//             if (error_word & FXMAC_RXSR_RXOVR_MASK)
//             {
//                 FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("Receive over run\r\n");
//                 FXmacMsgRecvHandler(arg);
//             }
//             if (error_word & FXMAC_RXSR_BUFFNA_MASK)
//             {
//                 FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("Receive buffer not available\r\n");
//                 FXmacMsgRecvHandler(arg);
//             }
//             break;
//         case FXMAC_MSG_SEND:
//             if (error_word & FXMAC_TXSR_HRESPNOK_MASK)
//             {
//                 FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("Transmit DMA error\r\n");
//                 FXmacHandleDmaTxError(instance_p);
//             }
//             if (error_word & FXMAC_TXSR_URUN_MASK)
//             {
//                 FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("Transmit under run\r\n");
//                 FXmacHandleTxErrors(instance_p);
//             }
//             if (error_word & FXMAC_TXSR_BUFEXH_MASK)
//             {
//                 FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("Transmit buffer exhausted\r\n");
//                 FXmacHandleTxErrors(instance_p);
//             }
//             if (error_word & FXMAC_TXSR_RXOVR_MASK)
//             {
//                 FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("Transmit retry excessed limits\r\n");
//                 FXmacHandleTxErrors(instance_p);
//             }
//             if (error_word & FXMAC_TXSR_FRAMERX_MASK)
//             {
//                 FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("Transmit collision\r\n");
//                 FXmacProcessSentBds(instance_p, txring);
//             }
//             break;
//         }
//     }
// }

void FXmacMsgLinkChange(void *args)
{
    u32 ctrl;
    u32 link, link_status;

    FXmacMsgCtrl *xmac_p;
    FXmacMsgLwipPort *instance_p;

    instance_p = (FXmacMsgLwipPort *)args;
    xmac_p = &instance_p->instance;

    FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("!!! Enter FXmacMsgLinkChange !!!");
    if (xmac_p->config.interface == FXMAC_MSG_PHY_INTERFACE_MODE_SGMII)
    {
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("xmac_p->mac_regs is %p", xmac_p->mac_regs);
        ctrl = FXMAC_MSG_READ(xmac_p, FXMAC_MSG_PCS_AN_LP);
        link = (ctrl & BIT(FXMAC_MSG_PCS_LINK_PARTNER_NEXT_PAGE_INDEX)) >>
               FXMAC_MSG_PCS_LINK_PARTNER_NEXT_PAGE_INDEX;

        switch (link)
        {
            case 0:
                FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("link status is down");
                link_status = FXMAC_MSG_LINKDOWN;
                break;
            case 1:
                FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("link status is up");
                link_status = FXMAC_MSG_LINKUP;
                break;
            default:
                FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("link status is error 0x%x \r\n", link);
                return;
        }

        if (link_status == FXMAC_MSG_LINKUP)
        {
            if (link_status != xmac_p->link_status)
            {
                xmac_p->link_status = FXMAC_MSG_NEGOTIATING;
                FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("need NEGOTIATING");
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
 * @name: phy_link_detect
 * @msg:  获取当前link status
 * @note:
 * @param {FXmacMsgCtrl} *fxmac_p
 * @param {u32} phy_addr
 * @return {*} 1 is link up , 0 is link down
 */
static u32 phy_link_detect(FXmacMsgCtrl *xmac_p, u32 phy_addr)
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

static u32 phy_autoneg_status(FXmacMsgCtrl *xmac_p, u32 phy_addr)
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

enum lwip_port_link_status FXmacMsgLwipPortLinkDetect(FXmacMsgLwipPort *instance_p)
{
    u32 phy_link_status;
    FXmacMsgCtrl *xmac_p = &instance_p->instance;

    if (xmac_p->is_ready != (u32)FT_COMPONENT_IS_READY)
    {
        return ETH_LINK_UNDEFINED;
    }

    phy_link_status = phy_link_detect(xmac_p, xmac_p->phy_address);

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
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_D("Ethernet Link down");
            return ETH_LINK_DOWN;
        case FXMAC_MSG_NEGOTIATING:
            if ((phy_link_status == FXMAC_MSG_LINKUP) &&
                phy_autoneg_status(xmac_p, xmac_p->phy_address))
            {
                err_t phy_ret;
                phy_ret = FXmacMsgPhyInit(xmac_p, xmac_p->config.speed, xmac_p->config.duplex,
                                          xmac_p->config.auto_neg, FXMAC_MSG_PHY_RESET_DISABLE);

                if (phy_ret != FT_SUCCESS)
                {
                    FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("FXmacPhyInit is error \r\n");
                    return ETH_LINK_DOWN;
                }
                FXmacMsgInterfaceConfig(xmac_p, 0);

                /* Initiate Phy setup to get link speed */
                xmac_p->link_status = FXMAC_MSG_LINKUP;
                FXMAC_MSG_LWIP_PORT_XMAC_PRINT_D("Ethernet Link up");
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
    FXmacMsgLwipPort *instance_p;
    FASSERT(xmac_netif_p != NULL);
    FASSERT(xmac_netif_p->state != NULL);

    instance_p = (FXmacMsgLwipPort *)(xmac_netif_p->state);

    xmac_p = &instance_p->instance;

    if (xmac_p->config.interface == FXMAC_MSG_PHY_INTERFACE_MODE_SGMII)
    {
        if (xmac_p->link_status == FXMAC_MSG_NEGOTIATING)
        {
            InterruptMask(xmac_p->config.queue_irq_num[0]);
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_D("FXmacMsgPhyReconnect: "
                                             "xmac_p->link_status is NEGOTIATING");
            /* 重新自协商 */
            err_t phy_ret;
            phy_ret = FXmacMsgPhyInit(xmac_p, xmac_p->config.speed, xmac_p->config.duplex,
                                      xmac_p->config.auto_neg, FXMAC_MSG_PHY_RESET_DISABLE);
            if (phy_ret != FT_SUCCESS)
            {
                FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("FXmacMsgPhyInit is error \r\n");
                InterruptUmask(xmac_p->config.queue_irq_num[0]);
                return ETH_LINK_DOWN;
            }
            FXmacMsgInterfaceConfig(xmac_p, 0);
            xmac_p->link_status = FXMAC_MSG_LINKUP;
            InterruptUmask(xmac_p->config.queue_irq_num[0]);
        }

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
    else if ((xmac_p->config.interface == FXMAC_MSG_PHY_INTERFACE_MODE_RMII) ||
             (xmac_p->config.interface == FXMAC_MSG_PHY_INTERFACE_MODE_RGMII))
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

static void FXmacMsgLwipPortIntrHandler(s32 vector, void *args)
{
    // printf("!!!!!!!!!!!!!!FXmacMsgLwipPortIntrHandler is here!!!!!!!!!!!!! \r\n");
    FXmacMsgIntrHandler(vector, args);
}

static void FXmacMsgSetupIsr(FXmacMsgLwipPort *instance_p)
{
    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(instance_p->instance.config.queue_irq_num[0], cpu_id);
    /* Setup callbacks */
    FXmacMsgSetHandler(&instance_p->instance, FXMAC_MSG_HANDLER_DMASEND,
                       FXmacMsgSendHandler, instance_p);
    FXmacMsgSetHandler(&instance_p->instance, FXMAC_MSG_HANDLER_DMARECV,
                       FXmacMsgRecvIsrHandler, instance_p);
    // FXmacMsgSetHandler(&instance_p->instance, FXMAC_MSG_HANDLER_ERROR, FXmacMsgErrorHandler, instance_p);
    FXmacMsgSetHandler(&instance_p->instance, FXMAC_MSG_HANDLER_LINKCHANGE,
                       FXmacMsgLinkChange, instance_p);

    InterruptSetPriority(instance_p->instance.config.queue_irq_num[0], IRQ_PRIORITY_VALUE_12);
    InterruptInstall(instance_p->instance.config.queue_irq_num[0],
                     FXmacMsgLwipPortIntrHandler, &instance_p->instance, "fxmac_msg");
    InterruptUmask(instance_p->instance.config.queue_irq_num[0]);
}

/*  init fxmac instance */

// static void FXmacMsgInitOnError(FXmacMsgLwipPort *instance_p)
// {
//     FXmacMsgCtrl *xmac_p;
//     u32 status = FT_SUCCESS;
//     xmac_p = &instance_p->instance;

//     /* set mac address */
//     status = FXmacSetMacAddress(xmac_p, (void *)(instance_p->hwaddr), 0);
//     if (status != FT_SUCCESS)
//     {
//         FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("In %s:Emac Mac Address set failed...\r\n", __func__);
//     }
// }

/* 根据用户配置（user_config）转化为硬件配置（xmac_config_p） */
static FError FXmacMsgLwipPortConfigConvert(FXmacMsgLwipPort *instance_p, FXmacMsgConfig *xmac_config_p)
{
    const FXmacMsgConfig *mac_config_p;
    FXmacMsgPhyInterface interface = FXMAC_MSG_PHY_INTERFACE_MODE_SGMII;

    mac_config_p = FXmacMsgLookupConfig(instance_p->xmac_port_config.instance_id);
    if (mac_config_p == NULL)
    {
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("FXmacLookupConfig is error , instance_id is "
                                         "%d",
                                         instance_p->xmac_port_config.instance_id);
        return FREERTOS_XMAC_MSG_INIT_ERROR;
    }

    memcpy(xmac_config_p, mac_config_p, sizeof(FXmacMsgConfig));

    switch (instance_p->xmac_port_config.interface)
    {
        case FXMAC_MSG_LWIP_PORT_INTERFACE_SGMII:
            interface = FXMAC_MSG_PHY_INTERFACE_MODE_SGMII;
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("SGMII select");
            break;
        case FXMAC_MSG_LWIP_PORT_INTERFACE_RMII:
            interface = FXMAC_MSG_PHY_INTERFACE_MODE_RMII;
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("RMII select");
            break;
        case FXMAC_MSG_LWIP_PORT_INTERFACE_RGMII:
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("RGMII select");
            interface = FXMAC_MSG_PHY_INTERFACE_MODE_RGMII;
            break;
        case FXMAC_MSG_LWIP_PORT_INTERFACE_USXGMII:
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("USXGMII select");
            instance_p->xmac_port_config.phy_speed = FXMAC_MSG_PHY_SPEED_10G;
            interface = FXMAC_MSG_PHY_INTERFACE_MODE_USXGMII;
            break;
        default:
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("update interface is error , interface is "
                                             "%d",
                                             instance_p->xmac_port_config.instance_id);
            return FREERTOS_XMAC_MSG_INIT_ERROR;
    }
    xmac_config_p->interface = interface;

    if (instance_p->xmac_port_config.autonegotiation)
    {
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("Auto-negotiation select");
        xmac_config_p->auto_neg = 1;
    }
    else
    {
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("Auto-negotiation not selected");
        xmac_config_p->auto_neg = 0;
    }

    switch (instance_p->xmac_port_config.phy_speed)
    {
        case FXMAC_MSG_PHY_SPEED_10M:
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("select FXMAC_MSG_PHY_SPEED_10M");
            xmac_config_p->speed = FXMAC_MSG_SPEED_10;
            break;
        case FXMAC_MSG_PHY_SPEED_100M:
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("select FXMAC_MSG_PHY_SPEED_100M");
            xmac_config_p->speed = FXMAC_MSG_SPEED_100;
            break;
        case FXMAC_MSG_PHY_SPEED_1000M:
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("select FXMAC_MSG_PHY_SPEED_1000M");
            xmac_config_p->speed = FXMAC_MSG_SPEED_1000;
            break;
        case FXMAC_MSG_PHY_SPEED_10G:
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("select FXMAC_MSG_PHY_SPEED_10G");
            xmac_config_p->speed = FXMAC_MSG_SPEED_10000;
            break;
        default:
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("setting speed is not valid , speed is %d",
                                             instance_p->xmac_port_config.phy_speed);
            return FREERTOS_XMAC_MSG_INIT_ERROR;
    }

    switch (instance_p->xmac_port_config.phy_duplex)
    {
        case FXMAC_MSG_PHY_HALF_DUPLEX:
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("select half duplex");
            xmac_config_p->duplex = 0;
            break;
        case FXMAC_MSG_PHY_FULL_DUPLEX:
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("select full duplex");
            xmac_config_p->duplex = 1;
            break;
    }

    return FT_SUCCESS;
}

/* 设置特性 */
void FXmacMsgFeatureSetOptions(u32 feature, FXmacMsgCtrl *xmac_p)
{
    u16 cmd_id, cmd_subid;
    cmd_id = FXMAC_MSG_CMD_SET;

    /* 巨帧 */
    if (feature & FXMAC_MSG_LWIP_PORT_CONFIG_JUMBO)
    {
        xmac_p->config.network_default_config |= FXMAC_MSG_JUMBO_ENABLE_OPTION;
        xmac_p->rx_buffer_len = FXMAC_MSG_MAX_FRAME_SIZE_JUMBO;
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("FXMAC_MSG_JUMBO_ENABLE_OPTION is ok");
    }

    /* 单播 */
    if (feature & FXMAC_MSG_LWIP_PORT_CONFIG_UNICAST_ADDRESS_FILITER)
    {
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("FXMAC_MSG_UNICAST_OPTION is ok");
    }

    /* 多播 */
    if (feature & FXMAC_MSG_LWIP_PORT_CONFIG_MULTICAST_ADDRESS_FILITER)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_MC;
        FXmacMsgSendMessage(xmac_p, cmd_id, cmd_subid, NULL, 0, 1);
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("FXMAC_MSG_MULTICAST_OPTION is ok");
    }

    /* 全帧 */
    if (feature & FXMAC_MSG_LWIP_PORT_CONFIG_COPY_ALL_FRAMES)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_PROMISE;
        FXmacMsgSendMessage(xmac_p, cmd_id, cmd_subid, NULL, 0, 0);
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("FXMAC_MSG_PROMISC_OPTION is ok");
    }

    /* 关闭FCS(帧校验序列)校验 */
    if (feature & FXMAC_MSG_LWIP_PORT_CONFIG_CLOSE_FCS_CHECK)
    {
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("FXMAC_MSG_FCS_STRIP_OPTION is ok");
    }
}

/*
1、将用户配置转化为硬件配置
2、初始化 FXmacMsgCtrl 结构体实例
3、设置功能选项
4、设置 MAC 地址
5、初始化 PHY
6、选择时钟
7、初始化接口
8、初始化DMA
9、初始化中断
*/
FError FXmacMsgLwipPortInit(FXmacMsgLwipPort *instance_p)
{
    FXmacMsgConfig mac_config;
    FXmacMsgCtrl *xmac_p;
    FError status;
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->xmac_port_config.instance_id < FXMAC_MSG_NUM);

    xmac_p = &instance_p->instance;
    FXMAC_MSG_LWIP_PORT_XMAC_PRINT_I("instance_id IS %d \r\n",
                                     instance_p->xmac_port_config.instance_id);

    /* 获取默认配置 */
    status = FXmacMsgLwipPortConfigConvert(instance_p, &mac_config);
    if (status != FT_SUCCESS)
    {
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("In %s: FXmacMsgLwipPortConfigConvert Convert "
                                         "to FXmacMsgConfig Failed....\r\n",
                                         __func__);
    }

    /* 配置初始化 */
    status = FXmacMsgCfgInitialize(xmac_p, &mac_config);
    if (status != FT_SUCCESS)
    {
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("In %s:Xmac Msg Configuration Failed....\r\n", __func__);
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
        status = FXmacMsgPhyInit(xmac_p, xmac_p->config.speed, xmac_p->config.duplex,
                                 xmac_p->config.auto_neg, FXMAC_MSG_PHY_RESET_ENABLE);
        if (status != FT_SUCCESS)
        {
            FXMAC_MSG_LWIP_PORT_XMAC_PRINT_W("FXmacMsgPhyInit is error \r\n");
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
 * @name: FXmacMsgLwipPortRx
 * @msg:  void *FXmacMsgLwipPortRx(FXmacMsgLwipPort *instance_p)
 * @note:
 * @param {FXmacMsgLwipPort} *instance_p
 * @return {*}
 */
void *FXmacMsgLwipPortRx(FXmacMsgLwipPort *instance_p)
{
    FASSERT(instance_p != NULL);
    struct pbuf *p;

    /* see if there is data to process */
    if (FXmacMsgPqQlength(&instance_p->recv_q) == 0)
    {
        return NULL;
    }
    /* return one packet from receive q */
    p = (struct pbuf *)FXmacMsgPqDequeue(&instance_p->recv_q);

    return p;
}

static FError FXmacMsgLwipPortOutput(FXmacMsgLwipPort *instance_p, struct pbuf *p)
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

FError FXmacMsgLwipPortTx(FXmacMsgLwipPort *instance_p, void *tx_buf)
{
    u32 freecnt;
    FXmacMsgBdRing *txring;
    FError ret = FT_SUCCESS;
    struct pbuf *p;
    FASSERT(instance_p != NULL);
    if (tx_buf == NULL)
    {
        return FREERTOS_XMAC_MSG_PARAM_ERROR;
    }

    p = tx_buf;
    sys_prot_t lev;
    lev = sys_arch_protect();
    /* check if space is available to send */
    freecnt = IsTxSpaceAvailable(instance_p);

    if (freecnt <= 5)
    {
        txring = &(FXMAC_MSG_GET_TXRING(instance_p->instance));
        FXmacMsgProcessSentBds(instance_p, txring);
    }

    if (IsTxSpaceAvailable(instance_p))
    {
        ret = FXmacMsgLwipPortOutput(instance_p, p);
    }
    else
    {
#if LINK_STATS
        lwip_stats.link.drop++;
#endif
        FXMAC_MSG_LWIP_PORT_XMAC_PRINT_E("pack dropped, no space\r\n");
        ret = FREERTOS_XMAC_MSG_NO_VALID_SPACE;
    }
    sys_arch_unprotect(lev);
    return ret;
}

FXmacMsgLwipPort *FXmacMsgLwipPortGetInstancePointer(u32 FXmacMsgLwipPortInstanceID)
{
    FASSERT(FXmacMsgLwipPortInstanceID < FXMAC_MSG_NUM);

    FXmacMsgLwipPort *instance_p;
    instance_p = &fxmac_msg_lwip_port_instance[FXmacMsgLwipPortInstanceID];
    return instance_p;
}


void FXmacMsgLwipPortStop(FXmacMsgLwipPort *instance_p)
{
    FASSERT(instance_p != NULL);

    /* need to add deinit interupt */
    /* step 1 close mac controler  */
    FXmacMsgStop(&instance_p->instance);
    /* step 2 free all pbuf */
    FreeTxRxPbufs(instance_p);
}

void FXmacMsgLwipPortStart(FXmacMsgLwipPort *instance_p)
{
    FASSERT(instance_p != NULL);

    /* start mac */
    FXmacMsgStart(&instance_p->instance);
}
