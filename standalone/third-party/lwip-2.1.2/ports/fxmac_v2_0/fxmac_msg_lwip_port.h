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
 * FilePath: fxmac_msg_lwip_port.h
 * Date: 2025-01-22 14:59:30
 * LastEditTime: 2025-01-22 14:59:30
 * Description:   This file is xmac msg portable code for lwip port input,output,status check.
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huangjin     2025/01/22            first release
 */
#ifndef FXMAC_MSG_LWIP_PORT_H
#define FXMAC_MSG_LWIP_PORT_H

#include "fxmac_msg.h"
#include "fxmac_msg_hw.h"
#include "fkernel.h"
#include "ferror_code.h"

#define FREERTOS_XMAC_MSG_INIT_ERROR     FT_CODE_ERR(ErrModPort, 0, 0x1)
#define FREERTOS_XMAC_MSG_PARAM_ERROR    FT_CODE_ERR(ErrModPort, 0, 0x2)
#define FREERTOS_XMAC_MSG_NO_VALID_SPACE FT_CODE_ERR(ErrModPort, 0, 0x3)


#ifdef __cplusplus
extern "C"
{
#endif

/* 缓冲区描述符(BD)空间大小 */
#define FXMAX_RX_BDSPACE_LENGTH               0x20000 /* default set 128KB*/
#define FXMAX_TX_BDSPACE_LENGTH               0x20000 /* default set 128KB*/

/* 数据包缓冲区(pbuf)的容量 */
#define FXMAX_RX_PBUFS_LENGTH                 128
#define FXMAX_TX_PBUFS_LENGTH                 128

#define FXMAX_MSG_MAX_HARDWARE_ADDRESS_LENGTH 6

/* configuration */
#define FXMAC_MSG_LWIP_PORT_CONFIG_JUMBO      BIT(0)
#define FXMAC_MSG_LWIP_PORT_CONFIG_MULTICAST_ADDRESS_FILITER \
    BIT(1) /* Allow multicast address filtering  */
#define FXMAC_MSG_LWIP_PORT_CONFIG_COPY_ALL_FRAMES BIT(2) /* enable copy all frames */
#define FXMAC_MSG_LWIP_PORT_CONFIG_CLOSE_FCS_CHECK BIT(3) /* close fcs check */
#define FXMAC_MSG_LWIP_PORT_CONFIG_UNICAST_ADDRESS_FILITER \
    BIT(5) /* Allow unicast address filtering  */

/* Phy */
#define FXMAC_MSG_PHY_SPEED_10M   10
#define FXMAC_MSG_PHY_SPEED_100M  100
#define FXMAC_MSG_PHY_SPEED_1000M 1000
#define FXMAC_MSG_PHY_SPEED_10G   10000

#define FXMAC_MSG_PHY_HALF_DUPLEX 0
#define FXMAC_MSG_PHY_FULL_DUPLEX 1

#define FXMAC_MSG_RECV_MAX_COUNT  10

/* Byte alignment of BDs */
#define BD_ALIGNMENT              (FXMAC_MSG_DMABD_MINIMUM_ALIGNMENT * 2)

/*  frame queue */
#define PQ_QUEUE_SIZE             4096

typedef struct
{
    uintptr data[PQ_QUEUE_SIZE];
    int head, tail, len;
} PqQueue;

typedef enum
{
    FXMAC_MSG_LWIP_PORT_INTERFACE_SGMII = 0,
    FXMAC_MSG_LWIP_PORT_INTERFACE_RMII,
    FXMAC_MSG_LWIP_PORT_INTERFACE_RGMII,
    FXMAC_MSG_LWIP_PORT_INTERFACE_USXGMII,
    FXMAC_MSG_LWIP_PORT_INTERFACE_LENGTH
} FXmacMsgLwipPortInterface;

typedef struct
{
    u8 rx_bdspace[FXMAX_RX_BDSPACE_LENGTH] __attribute__((aligned(256))); /* 接收bd 缓冲区 */
    u8 tx_bdspace[FXMAX_TX_BDSPACE_LENGTH] __attribute__((aligned(256))); /* 发送bd 缓冲区 */

    uintptr rx_pbufs_storage[FXMAX_RX_PBUFS_LENGTH];
    uintptr tx_pbufs_storage[FXMAX_TX_PBUFS_LENGTH];

} FXmacMsgNetifBuffer;

typedef struct
{
    u32 instance_id;
    FXmacMsgLwipPortInterface interface;
    u32 autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    u32 phy_speed;       /* FXMAC_MSG_PHY_SPEED_XXX */
    u32 phy_duplex;      /* FXMAC_MSG_PHY_XXX_DUPLEX */
} FXmacMsgLwipPortConfig;

typedef struct
{
    FXmacMsgCtrl instance;
    FXmacMsgLwipPortConfig xmac_port_config;

    FXmacMsgNetifBuffer buffer;

    /* queue to store overflow packets */
    PqQueue recv_q;
    PqQueue send_q;

    /* configuration */
    u32 feature;

    struct LwipPort *stack_pointer; /* Docking data stack data structure */
    u8 hwaddr[FXMAX_MSG_MAX_HARDWARE_ADDRESS_LENGTH];
    u32 recv_flg; /* Indicating how many receive interrupts have been triggered */
} FXmacMsgLwipPort;

FXmacMsgLwipPort *FXmacMsgLwipPortGetInstancePointer(u32 FXmacMsgLwipPortInstanceID);
FError FXmacMsgLwipPortInit(FXmacMsgLwipPort *instance_p);
void *FXmacMsgLwipPortRx(FXmacMsgLwipPort *instance_p);
FError FXmacMsgLwipPortTx(FXmacMsgLwipPort *instance_p, void *tx_buf);
void FXmacMsgLwipPortStop(FXmacMsgLwipPort *instance_p);
void FXmacMsgLwipPortStart(FXmacMsgLwipPort *instance_p);
void FXmacMsgResetDma(FXmacMsgLwipPort *instance_p);
enum lwip_port_link_status FXmacMsgPhyReconnect(struct LwipPort *xmac_netif_p);
void FXmacMsgRecvHandler(void *arg);
#ifdef __cplusplus
}
#endif

#endif // !
