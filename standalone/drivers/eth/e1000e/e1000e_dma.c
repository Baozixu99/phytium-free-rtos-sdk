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
 * FilePath: e1000e_dma.c
 * Date: 2025-01-03 14:46:52
 * LastEditTime: 2025-01-03 14:46:58
 * Description:  This file implements dma descriptor ring related functions.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2025/01/03    first release
 */

/***************************** Include Files *********************************/

#include <string.h>
#include "fassert.h"
#include "e1000e.h"
#include "e1000e_hw.h"
#include "fdrivers_port.h"
#include "fcache.h"
#include "lwip/pbuf.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define E1000E_DEBUG_TAG "E1000E-DMA"
#define E1000E_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(E1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define E1000E_WARN(format, ...) \
    FT_DEBUG_PRINT_W(E1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define E1000E_INFO(format, ...) \
    FT_DEBUG_PRINT_I(E1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define E1000E_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(E1000E_DEBUG_TAG, format, ##__VA_ARGS__)

#define INT_TO_POINTER(x) ((void *)(intptr_t)(x))
#define POINTER_TO_INT(x) ((uintptr)(x))

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/**
 * @name: FE1000ESetupRxDescRing
 * @msg: 配置E1000E的接收DMA描述符和缓冲区
 * @param {FE1000ECtrl *}instance_p 驱动控制数据
 * @return  {FError} RX DMA初始化的错误码信息，E1000E_SUCCESS 表示RX DMA初始化成功，其它返回值表示RX DMA初始化失败
 */
FError FE1000ESetupRxDescRing(FE1000ECtrl *instance_p)
{
    uintptr bar0_addr = instance_p->config.base_addr;
    uint32_t val;
    unsigned int n;
    struct pbuf *p;

    FASSERT(instance_p);

    /* 初始化描述符 */
    /* 将buffer 填入每个描述符 */
    for (n = 0; n < RX_DESCRIPTORS; n++)
    {
        p = pbuf_alloc(PBUF_RAW, FE1000E_MAX_FRAME_SIZE, PBUF_POOL);
        FCacheDCacheInvalidateRange((uintptr)p->payload, FE1000E_MAX_FRAME_SIZE);
        instance_p->rx[n].addr = (uintptr)p->payload;
        instance_p->rxb[n] = (uintptr)p;
    }

    /* RDBAL 写入描述符地址低位 */
    FE1000E_WRITEREG32(bar0_addr, E1000_RDBAL, (unsigned long)&instance_p->rx);
    /* RDBAH 将0写入描述符地址高位 */
    FE1000E_WRITEREG32(bar0_addr, E1000_RDBAH, 0);
    /* 写入 RDLEN  */
    FE1000E_WRITEREG32(bar0_addr, E1000_RDLEN, sizeof(instance_p->rx));
    /* 初始化 head tail */
    FE1000E_WRITEREG32(bar0_addr, E1000_RDH, 0);
    FE1000E_WRITEREG32(bar0_addr, E1000_RDT, 0);
    /* 使能 RXDCTL 第25 位 */
    FE1000E_WRITEREG32(bar0_addr, E1000_RXDCTL,
                       FE1000E_READREG32(bar0_addr, E1000_RXDCTL) | RXDCTL_ENABLE);
    /* 初始化 RCTL */
    val = FE1000E_READREG32(bar0_addr, E1000_RCTL);
    val |= RCTL_EN | RCTL_BAM | RCTL_BSIZE_2048 | RCTL_SECRC;
    FE1000E_WRITEREG32(bar0_addr, E1000_RCTL, val);
    /* 设置接收尾帧 */
    FE1000E_WRITEREG32(bar0_addr, E1000_RDT, RX_DESCRIPTORS - 1);
    return FE1000E_SUCCESS;
}

/**
 * @name: FE1000ESetupTxDescRing
 * @msg: 配置E1000E的发送DMA描述符和缓冲区
 * @param {FE1000ECtrl *}instance_p 驱动控制数据
 * @return  {FError} TX DMA初始化的错误码信息，E1000E_SUCCESS 表示TX DMA初始化成功，其它返回值表示TX DMA初始化失败
 */
FError FE1000ESetupTxDescRing(FE1000ECtrl *instance_p)
{
    uintptr bar0_addr = instance_p->config.base_addr;
    uint32_t val;

    FASSERT(instance_p);
    FE1000E_WRITEREG32(bar0_addr, E1000_TDBAL, (unsigned long)&instance_p->tx);
    FE1000E_WRITEREG32(bar0_addr, E1000_TDBAH, 0);
    FE1000E_WRITEREG32(bar0_addr, E1000_TDLEN, sizeof(instance_p->tx));
    FE1000E_WRITEREG32(bar0_addr, E1000_TDH, 0);
    FE1000E_WRITEREG32(bar0_addr, E1000_TDT, 0);
    FE1000E_WRITEREG32(bar0_addr, E1000_TXDCTL,
                       FE1000E_READREG32(bar0_addr, E1000_TXDCTL) | TXDCTL_ENABLE);
    val = FE1000E_READREG32(bar0_addr, E1000_TCTL);
    val |= TCTL_EN | TCTL_PSP | TCTL_CT_DEF | TCTL_COLD_DEF;
    FE1000E_WRITEREG32(bar0_addr, E1000_TCTL, val);
    FE1000E_WRITEREG32(bar0_addr, E1000_TIPG, TIPG_IPGT_DEF | TIPG_IPGR1_DEF | TIPG_IPGR2_DEF);

    return FE1000E_SUCCESS;
}

/**
 * @name: FE1000EPacketReceptionDone
 * @msg: Marks the completion of a packet reception and updates the RX descriptor index
 *       for the specified FE1000E device instance.
 * @param {FE1000ECtrl *}instance_p 驱动控制数据
 * @return: None
 */
void FE1000EPacketReceptionDone(FE1000ECtrl *instance_p)
{
    unsigned int idx = instance_p->rx_ring.desc_idx;
    instance_p->rx[idx].sta &= ~(0x01);
    instance_p->rx_ring.desc_idx = (instance_p->rx_ring.desc_idx + 1) % RX_DESCRIPTORS;
    FE1000E_WRITEREG32(instance_p->config.base_addr, E1000_RDT, idx);
}

/**
 * @name: FE1000ERecvFrame
 * @msg: 通过E1000E接收数据帧
 * @return  {FError} E1000E_SUCCESS 表示接收数据帧成功，其它返回值表示接收数据帧失败
 * @param {FE1000ECtrl} *instance_p 驱动控制数据
 * @note 调用函数前需要确保E1000E驱动初始化成功
 */
FError FE1000ERecvFrame(FE1000ECtrl *instance_p)
{
    // void *buf;
    uint16_t len;
    u32 rx_idx = instance_p->rx_ring.desc_idx;

    FASSERT(instance_p);

    if (!(instance_p->rx[rx_idx].sta & RDESC_STA_DD))
    {
        return FE1000E_ERR_RECEV_FAILED;
    }

    // buf = INT_TO_POINTER((uint32_t)instance_p->rx[rx_idx].addr);
    len = instance_p->rx[rx_idx].len;

    E1000E_DEBUG("rx[%d].sta: 0x%02hx", rx_idx, instance_p->rx[rx_idx].sta);
    E1000E_DEBUG("rx[%d].len: %d", rx_idx, instance_p->rx[rx_idx].len);

    E1000E_INFO("Recv buf is:");
    // FtDumpHexByte(((u8 *)(buf)), (u32)len);

    if (len <= 0)
    {
        E1000E_DEBUG("Invalid RX descriptor length: %hu", instance_p->rx[rx_idx].len);
        return FE1000E_ERR_RECEV_FAILED;
    }

    FE1000EPacketReceptionDone(instance_p);

    return FE1000E_SUCCESS;
}

/**
 * @name: FE1000ESendFrame
 * @msg: 通过FE1000E发送数据帧
 * @return {FError} FE1000E_SUCCESS 表示发送数据帧成功，其它返回值表示发送数据帧失败
 * @param {FE1000ECtrl *} instance_p 驱动控制数据
 * @param {u32} frame_len 数据帧长度
 * @note 调用函数前需要确保FE1000E驱动初始化成功
 */
FError FE1000ESendFrame(FE1000ECtrl *instance_p, u32 frame_len)
{
    FASSERT(instance_p);
    unsigned int idx = instance_p->tx_ring.desc_idx;

    if (0U == frame_len)
    {
        return FE1000E_SUCCESS;
    }

    E1000E_INFO("Send buf is:");
    // FtDumpHexByte((u8 *)(instance_p->txb[instance_p->tx_ring.desc_idx]), (u32)frame_len);

    E1000E_DEBUG("frame_len: %d", frame_len);
    instance_p->tx[idx].addr = POINTER_TO_INT(instance_p->txb[instance_p->tx_ring.desc_idx]);
    instance_p->tx[idx].len = frame_len;
    instance_p->tx[idx].cmd = TDESC_EOP | TDESC_RS | TDESC_IFCS;
    E1000E_DEBUG("tx[%d].sta: 0x%02hx", idx, instance_p->tx[idx].sta);
    E1000E_DEBUG("tx[%d].cmd: 0x%02hx", idx, instance_p->tx[idx].cmd);
    instance_p->tx_ring.desc_idx = (instance_p->tx_ring.desc_idx + 1) % TX_DESCRIPTORS;
    FE1000E_WRITEREG32(instance_p->config.base_addr, E1000_TDT, instance_p->tx_ring.desc_idx);
    DSB();
    // FCacheDCacheInvalidateRange(instance_p->tx[idx].addr, frame_len);
    /* 等待发送完成 */
    while (!(instance_p->tx[idx].sta))
    {
    }

    E1000E_DEBUG("tx[%d].sta: 0x%02hx", idx, instance_p->tx[idx].sta);
    E1000E_DEBUG("tx[%d].cmd: 0x%02hx", idx, instance_p->tx[idx].cmd);

    return (instance_p->tx[idx].sta & TDESC_STA_DD) ? FE1000E_SUCCESS : FE1000E_ERR_TRANS_FAILED;
}
