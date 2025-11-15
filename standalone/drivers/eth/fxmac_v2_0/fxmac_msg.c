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
 * FilePath: fxmac_msg.c
 * Date: 2024-10-28 14:46:52
 * LastEditTime: 2024-10-28 14:46:58
 * Description:  This file is for xmac msg driver .Functions in this file are the minimum required functions
 * for this driver.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2024/10/28  first release
 */

#include "fxmac_msg.h"
#include "ftypes.h"
#include "fxmac_msg_hw.h"
#include "stdio.h"
#include "fdrivers_port.h"
#include "fxmac_msg_common.h"
#include "fswap.h"
#include "faarch.h"


#define FXMAC_MSG_DEBUG_TAG "FXMAC_MSG"
#define FXMAC_MSG_PRINT_E(format, ...) \
    FT_DEBUG_PRINT_E(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_PRINT_I(format, ...) \
    FT_DEBUG_PRINT_I(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_PRINT_D(format, ...) \
    FT_DEBUG_PRINT_D(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_PRINT_W(format, ...) \
    FT_DEBUG_PRINT_W(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)

static void FXmacMsgIrqStubHandler(void)
{
    FASSERT_MSG(0, "Please register the interrupt callback function");
}

FError FXmacMsgCfgInitialize(FXmacMsgCtrl *xmac_msg_p, FXmacMsgConfig *config_p)
{
    FError ret = FT_SUCCESS;
    FASSERT(xmac_msg_p != NULL);
    FASSERT(config_p != NULL);

    xmac_msg_p->config = *config_p;
    xmac_msg_p->link_status = FXMAC_MSG_LINKDOWN;
    xmac_msg_p->is_ready = FT_COMPONENT_IS_READY;

    xmac_msg_p->rx_buffer_len = FXMAC_MSG_MAX_VLAN_FRAME_SIZE;

    /* 获取caps */
    xmac_msg_p->caps = config_p->caps;

    xmac_msg_p->send_irq_handler = (FXmacMsgIrqHandler)FXmacMsgIrqStubHandler;
    xmac_msg_p->send_args = NULL;

    xmac_msg_p->recv_irq_handler = (FXmacMsgIrqHandler)FXmacMsgIrqStubHandler;
    xmac_msg_p->recv_args = NULL;

    xmac_msg_p->error_irq_handler = (FXmacMsgErrorIrqHandler)FXmacMsgIrqStubHandler;
    xmac_msg_p->error_args = NULL;

    xmac_msg_p->link_change_handler = (FXmacMsgIrqHandler)FXmacMsgIrqStubHandler;
    xmac_msg_p->link_change_args = NULL;

    xmac_msg_p->restart_handler = (FXmacMsgIrqHandler)FXmacMsgIrqStubHandler;
    xmac_msg_p->restart_args = NULL;

    xmac_msg_p->mask = FXMAC_MSG_DEFUALT_INT_MASK;

    xmac_msg_p->mac_regs = config_p->msg.regfile;
    xmac_msg_p->msg_regs = config_p->msg.shmem;

    return ret;
}

void FXmacMsgResetHw(FXmacMsgCtrl *pdata)
{
    int q;
    u16 cmd_id, cmd_subid;
    FXmacMsgRingInfo ring;

    /* 禁用所有中断、清除所有中断状态位 */
    /* 清空tx尾指针、rx尾指针标号寄存器 */
    for (q = 0; q < pdata->queues_max_num; ++q)
    {
        FXMAC_MSG_WRITE(pdata, FXMAC_MSG_INT_DR(q), -1);
        FXMAC_MSG_WRITE(pdata, FXMAC_MSG_INT_SR(q), -1);
        FXMAC_MSG_WRITE(pdata, FXMAC_MSG_TX_PTR(q), 0);
        FXMAC_MSG_WRITE(pdata, FXMAC_MSG_RX_PTR(q), 0);
    }

    /* reset hw rx/tx enable */
    cmd_id = FXMAC_MSG_CMD_DEFAULT;
    cmd_subid = FXMAC_MSG_CMD_DEFAULT_RESET_HW;
    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, NULL, 0, 0);

    /* reset tx ring */
    memset(&ring, 0, sizeof(ring));
    ring.queue_num = pdata->queues_max_num;
    cmd_subid = FXMAC_MSG_CMD_DEFAULT_RESET_TX_QUEUE;
    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, (void *)(&ring), sizeof(ring), 0);

    /* reset rx ring */
    cmd_subid = FXMAC_MSG_CMD_DEFAULT_RESET_RX_QUEUE;
    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, (void *)(&ring), sizeof(ring), 1);
}


int FXmacMsgSetMacAddress(FXmacMsgCtrl *instance, const u8 *addr)
{
    u16 cmd_id;
    u16 cmd_subid;
    FXmacMsgMac para;

    memset(&para, 0, sizeof(para));
    cmd_id = FXMAC_MSG_CMD_SET;
    cmd_subid = FXMAC_MSG_CMD_SET_ADDR;
    para.addrl = cpu_to_le32(*((u32 *)addr));
    para.addrh = cpu_to_le16(*((u16 *)(addr + 4)));

    FXmacMsgSendMessage(instance, cmd_id, cmd_subid, (void *)(&para), sizeof(para), 1);

    return 0;
}


int FXmacMsgGetMacAddress(FXmacMsgCtrl *instance, u8 *addr)
{
    int index;
    u16 cmd_id, cmd_subid;
    FXmacMsgMac para;

    cmd_id = FXMAC_MSG_CMD_GET;
    cmd_subid = FXMAC_MSG_CMD_GET_ADDR;
    FXmacMsgSendMessage(instance, cmd_id, cmd_subid, NULL, 0, 1);

    index = FXmacMsgTxRingWrap(instance, instance->msg_ring.tx_msg_rd_tail);
    memcpy((void *)&para, (void *)(instance->msg_regs + FXMAC_MSG(index) + MSG_HDR_LEN),
           sizeof(FXmacMsgMac));

    addr[0] = para.addrl & 0xff;
    addr[1] = (para.addrl >> 8) & 0xff;
    addr[2] = (para.addrl >> 16) & 0xff;
    addr[3] = (para.addrl >> 24) & 0xff;
    addr[4] = para.addrh & 0xff;
    addr[5] = (para.addrh >> 8) & 0xff;

    return 0;
}

/* Phy */
void FXmacMsgMdioIdle(FXmacMsgCtrl *pdata)
{
    u32 val;

    /* wait for end of transfer */
    val = FXMAC_MSG_READ(pdata, FXMAC_MSG_NETWORK_STATUS);
    while (!(val & FXMAC_MSG_BIT(MIDLE)))
    {
        FDriverUdelay(1);
        val = FXMAC_MSG_READ(pdata, FXMAC_MSG_NETWORK_STATUS);
    }
}

int FXmacMsgMdioDataReadC22(FXmacMsgCtrl *pdata, int mii_id, int regnum)
{
    u16 data;

    FXMAC_MSG_WRITE(pdata, FXMAC_MSG_MDIO,
                    (FXMAC_MSG_BITS(CLAUSESEL, FXMAC_MSG_C22) |
                     FXMAC_MSG_BITS(MDCOPS, FXMAC_MSG_C22_READ) | FXMAC_MSG_BITS(PHYADDR, mii_id) |
                     FXMAC_MSG_BITS(REGADDR, regnum) | FXMAC_MSG_BITS(CONST, 2)));
    FXmacMsgMdioIdle(pdata);
    data = FXMAC_MSG_READ(pdata, FXMAC_MSG_MDIO) & 0xffff;
    FXmacMsgMdioIdle(pdata);
    return data;
}

int FXmacMsgMdioDataWriteC22(FXmacMsgCtrl *pdata, int mii_id, int regnum, u16 data)
{
    FXMAC_MSG_WRITE(pdata, FXMAC_MSG_MDIO,
                    (FXMAC_MSG_BITS(CLAUSESEL, FXMAC_MSG_C22) | FXMAC_MSG_BITS(MDCOPS, FXMAC_MSG_C22_WRITE) |
                     FXMAC_MSG_BITS(PHYADDR, mii_id) | FXMAC_MSG_BITS(REGADDR, regnum) |
                     FXMAC_MSG_BITS(VALUE, data) | FXMAC_MSG_BITS(CONST, 2)));
    FXmacMsgMdioIdle(pdata);

    return 0;
}

int FXmacMsgMdioDataReadC45(FXmacMsgCtrl *pdata, int mii_id, int devad, int regnum)
{
    u16 data;

    FXMAC_MSG_WRITE(pdata, FXMAC_MSG_MDIO,
                    (FXMAC_MSG_BITS(CLAUSESEL, FXMAC_MSG_C45) | FXMAC_MSG_BITS(MDCOPS, FXMAC_MSG_C45_ADDR) |
                     FXMAC_MSG_BITS(PHYADDR, mii_id) | FXMAC_MSG_BITS(REGADDR, devad & 0x1F) |
                     FXMAC_MSG_BITS(VALUE, regnum & 0xFFFF) | FXMAC_MSG_BITS(CONST, 2)));
    FXmacMsgMdioIdle(pdata);
    FXMAC_MSG_WRITE(pdata, FXMAC_MSG_MDIO,
                    (FXMAC_MSG_BITS(CLAUSESEL, FXMAC_MSG_C45) | FXMAC_MSG_BITS(MDCOPS, FXMAC_MSG_C45_READ) |
                     FXMAC_MSG_BITS(PHYADDR, mii_id) | FXMAC_MSG_BITS(REGADDR, devad & 0x1F) |
                     FXMAC_MSG_BITS(VALUE, regnum & 0xFFFF) | FXMAC_MSG_BITS(CONST, 2)));
    FXmacMsgMdioIdle(pdata);
    data = FXMAC_MSG_READ(pdata, FXMAC_MSG_MDIO) & 0xffff;
    FXmacMsgMdioIdle(pdata);
    return data;
}

int FXmacMsgMdioDataWriteC45(FXmacMsgCtrl *pdata, int mii_id, int devad, int regnum, u16 data)
{
    FXMAC_MSG_WRITE(pdata, FXMAC_MSG_MDIO,
                    (FXMAC_MSG_BITS(CLAUSESEL, FXMAC_MSG_C45) |
                     FXMAC_MSG_BITS(MDCOPS, FXMAC_MSG_C45_ADDR) | FXMAC_MSG_BITS(PHYADDR, mii_id) |
                     FXMAC_MSG_BITS(REGADDR, (regnum >> 16) & 0x1F) |
                     FXMAC_MSG_BITS(VALUE, regnum & 0xFFFF) | FXMAC_MSG_BITS(CONST, 2)));
    FXmacMsgMdioIdle(pdata);
    FXMAC_MSG_WRITE(pdata, FXMAC_MSG_MDIO,
                    (FXMAC_MSG_BITS(CLAUSESEL, FXMAC_MSG_C45) |
                     FXMAC_MSG_BITS(MDCOPS, FXMAC_MSG_C45_WRITE) | FXMAC_MSG_BITS(PHYADDR, mii_id) |
                     FXMAC_MSG_BITS(REGADDR, (regnum >> 16) & 0x1F) |
                     FXMAC_MSG_BITS(VALUE, data) | FXMAC_MSG_BITS(CONST, 2)));
    FXmacMsgMdioIdle(pdata);

    return 0;
}

/* promise */
int FXmacMsgEnablePromise(FXmacMsgCtrl *pdata, int enable)
{
    u16 cmd_id, cmd_subid;
    u8 rxcsum = 0;

    cmd_id = FXMAC_MSG_CMD_SET;
    if (enable)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_PROMISE;
    }
    else
    {
        cmd_subid = FXMAC_MSG_CMD_SET_DISABLE_PROMISE;
        if (pdata->config.network_default_config & FXMAC_MSG_RX_CHKSUM_ENABLE_OPTION)
        {
            rxcsum = 1;
        }
    }

    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, (void *)(&rxcsum), 1, 1);

    return 0;
}

/* multicast */
int FXmacMsgEnableMulticast(FXmacMsgCtrl *pdata, int enable)
{
    u16 cmd_id, cmd_subid;

    cmd_id = FXMAC_MSG_CMD_SET;
    if (enable)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_MC;
    }
    else
    {
        cmd_subid = FXMAC_MSG_CMD_SET_DISABLE_MC;
    }

    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, NULL, 0, 1);
    return 0;
}

/* hash */
int FXmacMsgSetMcHash(FXmacMsgCtrl *pdata, unsigned int *mc_filter)
{
    u16 cmd_id, cmd_subid;
    FXmacMsgMcInfo para;

    memset(&para, 0, sizeof(para));
    cmd_id = FXMAC_MSG_CMD_SET;
    cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_HASH_MC;
    para.mc_bottom = (u32)mc_filter[0];
    para.mc_top = (u32)mc_filter[1];
    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, (void *)(&para), sizeof(para), 1);

    return 0;
}

/* rxcsum */
int FXmacMsgEnableRxcsum(FXmacMsgCtrl *pdata, int enable)
{
    u16 cmd_id, cmd_subid;

    cmd_id = FXMAC_MSG_CMD_SET;
    if (enable)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_RXCSUM;
    }
    else
    {
        cmd_subid = FXMAC_MSG_CMD_SET_DISABLE_RXCSUM;
    }

    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, NULL, 0, 1);

    return 0;
}

/* txcsum */
int FXmacMsgEnableTxcsum(FXmacMsgCtrl *pdata, int enable)
{
    u16 cmd_id, cmd_subid;

    cmd_id = FXMAC_MSG_CMD_SET;
    if (enable)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_TXCSUM;
    }
    else
    {
        cmd_subid = FXMAC_MSG_CMD_SET_DISABLE_TXCSUM;
    }

    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, NULL, 0, 1);

    return 0;
}

int FXmacMsgEnableMdio(FXmacMsgCtrl *pdata, int enable)
{
    u16 cmd_id, cmd_subid;

    cmd_id = FXMAC_MSG_CMD_SET;
    if (enable)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_MDIO;
    }
    else
    {
        cmd_subid = FXMAC_MSG_CMD_SET_DISABLE_MDIO;
    }

    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, NULL, 0, 1);

    return 0;
}

int FXmacMsgEnableAutoneg(FXmacMsgCtrl *pdata, int enable)
{
    u16 cmd_id, cmd_subid;

    cmd_id = FXMAC_MSG_CMD_SET;
    if (enable)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_AUTONEG;
    }
    else
    {
        cmd_subid = FXMAC_MSG_CMD_SET_DISABLE_AUTONEG;
    }

    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, NULL, 0, 1);

    pdata->config.auto_neg = enable;
    return 0;
}

int FXmacMsgEnablePause(FXmacMsgCtrl *pdata, int enable)
{
    u16 cmd_id, cmd_subid;

    cmd_id = FXMAC_MSG_CMD_SET;
    if (enable)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_PAUSE;
    }
    else
    {
        cmd_subid = FXMAC_MSG_CMD_SET_DISABLE_PAUSE;
    }

    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, NULL, 0, 1);

    return 0;
}

int FXmacMsgEnableNetwork(FXmacMsgCtrl *pdata, int enable)
{
    u16 cmd_id, cmd_subid;

    cmd_id = FXMAC_MSG_CMD_SET;
    if (enable)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_NETWORK;
    }
    else
    {
        cmd_subid = FXMAC_MSG_CMD_SET_DISABLE_NETWORK;
    }

    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, NULL, 0, 1);

    return 0;
}

void FXmacMsgTxStart(FXmacMsgQueue *queue)
{
    FXmacMsgCtrl *pdata = queue->pdata;

    FXMAC_MSG_WRITE(pdata, FXMAC_MSG_TX_PTR(queue->index), queue->tx_tail);
    queue->tx_xmit_more = 0;
}


int FXmacMsgInitHw(FXmacMsgCtrl *instance_p, u8 *mac_address_ptr)
{
    u16 cmd_id, cmd_subid;
    FXmacMsgDmaInfo dma;
    FXmacMsgEthInfo eth;
    u8 mdc;

    /* 使能MDIO */
    cmd_id = FXMAC_MSG_CMD_SET;
    cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_MDIO;
    FXmacMsgSendMessage(instance_p, cmd_id, cmd_subid, NULL, 0, 0);

    /* 设置MAC地址 */
    FXmacMsgSetMacAddress(instance_p, mac_address_ptr);

    FXMAC_MSG_PRINT_D("Entry FXmacMsgInitHw !!!");
    FXMAC_MSG_PRINT_D("instance_p->caps = 0x%x", instance_p->caps);
    FXMAC_MSG_PRINT_D("instance_p->config.caps = 0x%x", instance_p->config.caps);
    FXMAC_MSG_PRINT_D("instance_p->config.network_default_config = 0x%x",
                      instance_p->config.network_default_config);
    cmd_id = FXMAC_MSG_CMD_SET;

    /* 设置帧长度 */
    if (instance_p->config.network_default_config & FXMAC_MSG_JUMBO_ENABLE_OPTION)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_JUMBO;
    }
    else
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_1536_FRAMES;
    }
    FXmacMsgSendMessage(instance_p, cmd_id, cmd_subid, NULL, 0, 0);

    /* 使能混杂模式 */
    if (instance_p->config.network_default_config & FXMAC_MSG_PROMISC_OPTION)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_PROMISE;
        FXmacMsgSendMessage(instance_p, cmd_id, cmd_subid, NULL, 0, 0);
    }

    /* 使能rxcsum */
    if (instance_p->config.network_default_config & FXMAC_MSG_RX_CHKSUM_ENABLE_OPTION)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_RXCSUM;
        FXmacMsgSendMessage(instance_p, cmd_id, cmd_subid, NULL, 0, 0);
    }

    /* 使能广播 */
    if (instance_p->config.network_default_config & FXMAC_MSG_BROADCAST_OPTION)
    {
        cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_BC;
    }
    else
    {
        cmd_subid = FXMAC_MSG_CMD_SET_DISABLE_BC;
    }
    FXmacMsgSendMessage(instance_p, cmd_id, cmd_subid, NULL, 0, 0);

    /* 使能流控 */
    cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_PAUSE;
    FXmacMsgSendMessage(instance_p, cmd_id, cmd_subid, NULL, 0, 0);

    /* 使能跳过CRC校验 */
    cmd_subid = FXMAC_MSG_CMD_SET_ENABLE_STRIPCRC;
    FXmacMsgSendMessage(instance_p, cmd_id, cmd_subid, NULL, 0, 0);

    /* 配置DMA */
    memset(&dma, 0, sizeof(dma));
    cmd_subid = FXMAC_MSG_CMD_SET_DMA;
    dma.dma_burst_length = instance_p->config.dma_brust_length;
    FXMAC_MSG_PRINT_D("instance_p->config.dma_brust_length = %d", instance_p->config.dma_brust_length);
    FXMAC_MSG_PRINT_D("dma_burst_length = %d", dma.dma_burst_length);
    FXMAC_MSG_PRINT_D("instance_p->dma_data_width = %d", instance_p->dma_data_width);
    FXMAC_MSG_PRINT_D("instance_p->dma_addr_width = %d", instance_p->dma_addr_width);
    if (instance_p->dma_addr_width)
    {
        dma.hw_dma_cap |= HW_DMA_CAP_64B;
    }
    if (instance_p->config.network_default_config & FXMAC_MSG_TX_CHKSUM_ENABLE_OPTION)
    {
        dma.hw_dma_cap |= HW_DMA_CAP_CSUM;
    }
    if (instance_p->dma_data_width == FXMAC_MSG_DBW32)
    {
        dma.hw_dma_cap |= HW_DMA_CAP_DDW32;
    }
    if (instance_p->dma_data_width == FXMAC_MSG_DBW64)
    {
        dma.hw_dma_cap |= HW_DMA_CAP_DDW64;
    }
    if (instance_p->dma_data_width == FXMAC_MSG_DBW128)
    {
        dma.hw_dma_cap |= HW_DMA_CAP_DDW128;
    }
    FXmacMsgSendMessage(instance_p, cmd_id, cmd_subid, (void *)&dma, sizeof(dma), 0);

    /* 设置分频系数 */
    cmd_subid = FXMAC_MSG_CMD_SET_MDC;
    mdc = FXMAC_MSG_CLK_DIV96;
    FXmacMsgSendMessage(instance_p, cmd_id, cmd_subid, (void *)(&mdc), sizeof(mdc), 0);

    /* 配置eth类型 */
    memset(&eth, 0, sizeof(eth));
    cmd_subid = FXMAC_MSG_CMD_SET_ETH_MATCH;
    eth.index = 0;
    eth.etype = (uint16_t)ETH_P_IP;
    FXmacMsgSendMessage(instance_p, cmd_id, cmd_subid, (void *)&eth, sizeof(eth), 1);

    /* 初始化环形队列 */
    FXmacMsgInitRingHw(instance_p);

    return 0;
}


int FXmacMsgInitRingHw(FXmacMsgCtrl *pdata)
{
    u16 cmd_id, cmd_subid;
    FXmacMsgRingInfo rxring;
    FXmacMsgRingInfo txring;
    FXmacMsgRxBufInfo rxbuf;
    FXmacMsgQueue *queue;
    u32 q;

    memset(&rxring, 0, sizeof(rxring));
    memset(&txring, 0, sizeof(txring));
    memset(&rxbuf, 0, sizeof(rxbuf));
    cmd_id = FXMAC_MSG_CMD_SET;
    cmd_subid = FXMAC_MSG_CMD_SET_INIT_TX_RING;
    txring.queue_num = 1;
    rxring.queue_num = 1;
    txring.hw_dma_cap |= HW_DMA_CAP_64B;
    rxring.hw_dma_cap |= HW_DMA_CAP_64B;
    for (q = 0, queue = pdata->queues; q < 1; ++q, ++queue)
    {
        txring.addr[q] = queue->tx_ring_addr;
        rxring.addr[q] = queue->rx_ring_addr;
    }

    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, (void *)(&txring), sizeof(txring), 0);

    cmd_id = FXMAC_MSG_CMD_SET;
    cmd_subid = FXMAC_MSG_CMD_SET_DMA_RX_BUFSIZE;
    rxbuf.queue_num = pdata->queues_num;
    rxbuf.buffer_size = pdata->rx_buffer_len / 64;
    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, (void *)(&rxbuf), sizeof(rxbuf), 0);

    cmd_id = FXMAC_MSG_CMD_SET;
    cmd_subid = FXMAC_MSG_CMD_SET_INIT_RX_RING;
    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, (void *)(&rxring), sizeof(rxring), 0);

    return 0;
}

int FXmacMsgGetFeatureAll(FXmacMsgCtrl *pdata)
{
    u16 cmd_id, cmd_subid;
    int index;
    FXmacMsgFeature para;

    memset(&para, 0, sizeof(para));
    cmd_id = FXMAC_MSG_CMD_GET;
    cmd_subid = FXMAC_MSG_CMD_GET_CAPS;
    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, NULL, 0, 1);

    index = FXmacMsgTxRingWrap(pdata, pdata->msg_ring.tx_msg_rd_tail);

    memcpy((void *)&para, (void *)(pdata->msg_regs + FXMAC_MSG(index) + MSG_HDR_LEN), 8);
    memcpy(((uint8_t *)&para) + 8,
           (void *)(pdata->msg_regs + FXMAC_MSG(index) + MSG_HDR_LEN + 8), 2);

    pdata->queues_max_num = para.queue_num;
    if (para.dma_addr_width)
    {
        pdata->dma_addr_width = 64;
    }
    else
    {
        pdata->dma_addr_width = 32;
    }
    pdata->dma_data_width = para.dma_data_width;
    pdata->max_rx_fs = para.max_rx_fs;
    pdata->tx_bd_prefetch = (2 << (para.tx_bd_prefetch - 1)) * sizeof(FXmacMsgDmaDesc);
    pdata->rx_bd_prefetch = (2 << (para.rx_bd_prefetch - 1)) * sizeof(FXmacMsgDmaDesc);

    FXMAC_MSG_PRINT_D("irq_read_clear=%d, dma_data_width=%d, dma_addr_width=%d, "
                      "tx_pkt_buffer=%d, rx_pkt_buffer=%d, pbuf_lso=%d, queue_num=%d, "
                      "tx_bd_prefetch=%d, rx_bd_prefetch=%d, max_rx_fs=%d",
                      para.irq_read_clear, para.dma_data_width, para.dma_addr_width,
                      para.tx_pkt_buffer, para.rx_pkt_buffer, para.pbuf_lso, para.queue_num,
                      para.tx_bd_prefetch, para.rx_bd_prefetch, para.max_rx_fs);

    FXMAC_MSG_PRINT_D("feature qnum=%d, daw=%d, dbw=%d, rxfs=%d, rxbd=%d, txbd=%d",
                      pdata->queues_num, pdata->dma_addr_width, pdata->dma_data_width,
                      pdata->max_rx_fs, pdata->rx_bd_prefetch, pdata->tx_bd_prefetch);

    return 0;
}

/**
 * Start the Ethernet controller as follows:
 *   - Enable transmitter if FXMAC_TRANSMIT_ENABLE_OPTION is set
 *   - Enable receiver if FXMAC_RECEIVER_ENABLE_OPTION is set
 *   - Start the SG DMA send and receive channels and enable the device
 *     interrupt
 *
 * @param instance_p is a pointer to the instance to be worked on.
 *
 * @return N/A
 *
 * @note
 * Hardware is configured with scatter-gather DMA, the driver expects to start
 * the scatter-gather channels and expects that the user has previously set up
 * the buffer descriptor lists.
 *
 * This function makes use of internal resources that are shared between the
 * Start, Stop, and Set/ClearOptions functions. So if one task might be setting
 * device options while another is trying to start the device, the user is
 * required to provide protection of this shared data (typically using a
 * semaphore).
 *
 * This function must not be preempted by an interrupt that may service the
 * device.
 *
 */
void FXmacMsgStart(FXmacMsgCtrl *instance_p)
{
    u16 cmd_id, cmd_subid;

    /* Assert bad arguments and conditions */
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == (u32)FT_COMPONENT_IS_READY);

    /* clear any existed int status */
    FXmacMsgDisableIrq(instance_p, 0, FXMAC_MSG_IXR_ALL_MASK);

    /* Enable transmitter if not already enabled */
    if ((instance_p->config.network_default_config & (u32)FXMAC_MSG_TRANSMITTER_ENABLE_OPTION) != 0x00000000U)
    {
        cmd_id = FXMAC_MSG_CMD_SET;
        cmd_subid = FXMAC_MSG_CMD_SET_INIT_TX_ENABLE_TRANSMIT;
        FXmacMsgSendMessage(instance_p, cmd_id, cmd_subid, NULL, 0, 1);
    }

    /* Enable receiver if not already enabled */
    if ((instance_p->config.network_default_config & FXMAC_MSG_RECEIVER_ENABLE_OPTION) != 0x00000000U)
    {
        cmd_id = FXMAC_MSG_CMD_SET;
        cmd_subid = FXMAC_MSG_CMD_SET_INIT_RX_ENABLE_RECEIVE;
        FXmacMsgSendMessage(instance_p, cmd_id, cmd_subid, NULL, 0, 1);
    }

    /* 第一次使用尾指针时，指向描述符环尾部 */
    /*修改尾指针需要加内存屏障指令*/
    DSB();
    FXMAC_MSG_WRITE(instance_p, FXMAC_MSG_RX_PTR(0), (BIT(7) - 1));

    /* Enable TX and RX interrupt */
    FXmacMsgEnableIrq(instance_p, 0, instance_p->mask);
    FXMAC_MSG_PRINT_D("FXmacMsgStart: mask = 0x%x", instance_p->mask);
    /* Mark as started */
    instance_p->is_started = FT_COMPONENT_IS_STARTED;

    return;
}

/**
 * Gracefully stop the Ethernet MAC as follows:
 *   - Disable all interrupts from this device
 *   - Stop DMA channels
 *   - Disable the tansmitter and receiver
 *
 * Device options currently in effect are not changed.
 *
 * This function will disable all interrupts. Default interrupts settings that
 * had been enabled will be restored when FXmacMsgStart() is called.
 *
 * @param instance_p is a pointer to the instance to be worked on.
 *
 * @note
 * This function makes use of internal resources that are shared between the
 * Start, Stop, Setoptions, and Clearoptions functions. So if one task might be
 * setting device options while another is trying to start the device, the user
 * is required to provide protection of this shared data (typically using a
 * semaphore).
 *
 * Stopping the DMA channels causes this function to block until the DMA
 * operation is complete.
 *
 */
void FXmacMsgStop(FXmacMsgCtrl *instance_p)
{
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == (u32)FT_COMPONENT_IS_READY);

    /* Disable all interrupts */
    FXMAC_MSG_WRITE(instance_p, FXMAC_MSG_INT_SR(instance_p->queues[0].index), FXMAC_MSG_IXR_ALL_MASK);
    FXmacMsgDisableIrq(instance_p, instance_p->queues[0].index, FXMAC_MSG_IXR_ALL_MASK);

    /* Disable the receiver & transmitter */
    FXmacMsgEnableNetwork(instance_p, 0);

    /* Mark as stopped */
    instance_p->is_started = 0U;
}

/**
 * This function sets the start address of the transmit/receive buffer queue.
 *
 * @param   instance_p is a pointer to the instance to be worked on.
 * @param   queue_p is the address of the Queue to be written
 * @param   queue_num is the Buffer Queue Index
 * @param   direction indicates Transmit/Receive
 *
 * @note
 * The buffer queue addresses has to be set before starting the transfer, so
 * this function has to be called in prior to FXmacMsgStart()
 *
 */
void FXmacMsgSetQueuePtr(FXmacMsgCtrl *instance_p, uintptr queue_p, u8 queue_num, u32 direction)
{
    /* Assert bad arguments and conditions */
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == (u32)FT_COMPONENT_IS_READY);

    /* If already started, then there is nothing to do */
    if (instance_p->is_started == (u32)FT_COMPONENT_IS_STARTED)
    {
        return;
    }

    u16 cmd_id, cmd_subid;
    FXmacMsgRingInfo rxring;
    FXmacMsgRingInfo txring;
    u32 q = 0;

    memset(&rxring, 0, sizeof(rxring));
    memset(&txring, 0, sizeof(txring));

    if (queue_num == 0x00U)
    {
        if (direction == FXMAC_MSG_SEND)
        {
            txring.queue_num = queue_num + 1;
            txring.hw_dma_cap = HW_DMA_CAP_64B;
            txring.addr[q] = queue_p | (((queue_p == (uintptr)0)) ? 1 : 0);
            cmd_id = FXMAC_MSG_CMD_SET;
            cmd_subid = FXMAC_MSG_CMD_SET_INIT_TX_RING;
            FXMAC_MSG_PRINT_D("FXmacMsgSetQueuePtr: tx queue_p = 0x%x", queue_p);
            FXmacMsgSendMessage(instance_p, cmd_id, cmd_subid, (void *)(&txring),
                                sizeof(txring), 0);
        }
        else
        {
            rxring.queue_num = queue_num + 1;
            rxring.hw_dma_cap = HW_DMA_CAP_64B;
            rxring.addr[q] = queue_p | (((queue_p == (uintptr)0)) ? 1 : 0);
            cmd_id = FXMAC_MSG_CMD_SET;
            cmd_subid = FXMAC_MSG_CMD_SET_INIT_RX_RING;
            FXMAC_MSG_PRINT_D("FXmacMsgSetQueuePtr: rx queue_p = 0x%x", queue_p);
            FXmacMsgSendMessage(instance_p, cmd_id, cmd_subid, (void *)(&rxring),
                                sizeof(rxring), 0);
        }
    }
}

FError FXmacMsgSetHash(FXmacMsgCtrl *intance_p, void *mac_address)
{
    u8 *Aptr = (u8 *)(void *)mac_address;
    u8 Temp1, Temp2, Temp3, Temp4, Temp5, Temp6, Temp7, Temp8;
    u32 Result;
    FError Status;
    u32 hashes[2] = {0, 0};

    FASSERT(intance_p != NULL);
    FASSERT(mac_address != NULL);
    FASSERT(intance_p->is_ready == (u32)FT_COMPONENT_IS_READY);

    /* Be sure device has been stopped */
    if (intance_p->is_started == (u32)FT_COMPONENT_IS_STARTED)
    {
        Status = (FError)(FXMAC_MSG_ERR_MAC_IS_PROCESSING);
    }
    else
    {
        Temp1 = (*(Aptr + 0)) & 0x3FU;
        Temp2 = ((*(Aptr + 0) >> 6U) & 0x03U) | ((*(Aptr + 1) & 0x0FU) << 2U);
        Temp3 = ((*(Aptr + 1) >> 4U) & 0x0FU) | ((*(Aptr + 2) & 0x3U) << 4U);
        Temp4 = ((*(Aptr + 2) >> 2U) & 0x3FU);
        Temp5 = (*(Aptr + 3)) & 0x3FU;
        Temp6 = ((*(Aptr + 3) >> 6U) & 0x03U) | ((*(Aptr + 4) & 0x0FU) << 2U);
        Temp7 = ((*(Aptr + 4) >> 4U) & 0x0FU) | ((*(Aptr + 5) & 0x03U) << 4U);
        Temp8 = ((*(Aptr + 5) >> 2U) & 0x3FU);

        Result = (u32)((u32)Temp1 ^ (u32)Temp2 ^ (u32)Temp3 ^ (u32)Temp4 ^ (u32)Temp5 ^
                       (u32)Temp6 ^ (u32)Temp7 ^ (u32)Temp8);

        if (Result >= (u32)FXMAC_MSG_MAX_HASH_BITS)
        {
            Status = (FError)(FXMAC_MSG_ERR_INVALID_PARAM);
        }
        else
        {
            if (Result < (u32)32)
            {
                hashes[0] |= (u32)(0x00000001U << Result);
            }
            else
            {
                hashes[1] |= (u32)(0x00000001U << (u32)(Result - (u32)32));
            }
            FXmacMsgSetMcHash(intance_p, hashes);
            Status = FT_SUCCESS;
        }
    }

    return Status;
}

FError FXmacMsgDeleteHash(FXmacMsgCtrl *intance_p, void *mac_address)
{
    u8 *Aptr = (u8 *)(void *)mac_address;
    u8 Temp1, Temp2, Temp3, Temp4, Temp5, Temp6, Temp7, Temp8;
    u32 Result;
    FError Status;
    u32 hashes[2] = {0, 0};

    FASSERT(intance_p != NULL);
    FASSERT(mac_address != NULL);
    FASSERT(intance_p->is_ready == (u32)FT_COMPONENT_IS_READY);

    /* Be sure device has been stopped */
    if (intance_p->is_started == (u32)FT_COMPONENT_IS_STARTED)
    {
        Status = (FError)(FXMAC_MSG_ERR_MAC_IS_PROCESSING);
    }
    else
    {
        Temp1 = (*(Aptr + 0)) & 0x3FU;
        Temp2 = ((*(Aptr + 0) >> 6U) & 0x03U) | ((*(Aptr + 1) & 0x0FU) << 2U);
        Temp3 = ((*(Aptr + 1) >> 4U) & 0x0FU) | ((*(Aptr + 2) & 0x03U) << 4U);
        Temp4 = ((*(Aptr + 2) >> 2U) & 0x3FU);
        Temp5 = (*(Aptr + 3)) & 0x3FU;
        Temp6 = ((*(Aptr + 3) >> 6U) & 0x03U) | ((*(Aptr + 4) & 0x0FU) << 2U);
        Temp7 = ((*(Aptr + 4) >> 4U) & 0x0FU) | ((*(Aptr + 5) & 0x03U) << 4U);
        Temp8 = ((*(Aptr + 5) >> 2U) & 0x3FU);

        Result = (u32)((u32)Temp1 ^ (u32)Temp2 ^ (u32)Temp3 ^ (u32)Temp4 ^ (u32)Temp5 ^
                       (u32)Temp6 ^ (u32)Temp7 ^ (u32)Temp8);

        if (Result >= (u32)FXMAC_MSG_MAX_HASH_BITS)
        {
            Status = (FError)(FXMAC_MSG_ERR_INVALID_PARAM);
        }
        else
        {
            if (Result < (u32)32)
            {
                hashes[0] &= (u32)(~(0x00000001U << Result));
            }
            else
            {
                hashes[1] &= (u32)(~(0x00000001U << (u32)(Result - (u32)32)));
            }
            FXmacMsgSetMcHash(intance_p, hashes);
            Status = FT_SUCCESS;
        }
    }
    return Status;
}
