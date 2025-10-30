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
 * FilePath: fxmac_msg_os.h
 * Date: 2025-04-14 16:33:19
 * LastEditTime: 2025-04-14 16:33:19
 * Description:  This file is for xmac v2 driver.Functions in this file are the minimum required functions for drivers.
 *
 * Modify History:
 *  Ver   Who        Date                   Changes
 * ----- ------    --------     --------------------------------------
 *  1.0  huangjin  2025/06/24            first release
 */

#ifndef FXMAC_MSG_OS_H
#define FXMAC_MSG_OS_H

#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include "fxmac_msg.h"
#include "fkernel.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FREERTOS_XMAC_MSG_INIT_ERROR FT_CODE_ERR(ErrModPort, 0, 0x1)
#define FREERTOS_XMAC_MSG_PARAM_ERROR FT_CODE_ERR(ErrModPort, 0, 0x2)
#define FREERTOS_XMAC_MSG_NO_VALID_SPACE FT_CODE_ERR(ErrModPort, 0, 0x3)

#define FXMAC_MSG_RX_BDSPACE_LENGTH    0x20000 /* default set 128KB*/
#define FXMAC_MSG_TX_BDSPACE_LENGTH    0x20000 /* default set 128KB*/

#define FXMAC_MSG_TX_PBUFS_LENGTH       128
#define FXMAC_MSG_RX_PBUFS_LENGTH       128

#define FXMAC_MSG_MAX_HARDWARE_ADDRESS_LENGTH 6

#define XMAC_PHY_RESET_ENABLE 1
#define XMAC_PHY_RESET_DISABLE 0

/* configuration */
#define FXMAC_MSG_OS_CONFIG_JUMBO  BIT(0)
#define FXMAC_MSG_OS_CONFIG_MULTICAST_ADDRESS_FILITER  BIT(1) /* Allow multicast address filtering  */
#define FXMAC_MSG_OS_CONFIG_COPY_ALL_FRAMES BIT(2) /* enable copy all frames */
#define FXMAC_MSG_OS_CONFIG_CLOSE_FCS_CHECK BIT(3) /* close fcs check */
#define FXMAC_MSG_OS_CONFIG_UNICAST_ADDRESS_FILITER BIT(5) /* Allow unicast address filtering  */
/* Phy */
#define FXMAC_MSG_PHY_SPEED_10M    10
#define FXMAC_MSG_PHY_SPEED_100M    100
#define FXMAC_MSG_PHY_SPEED_1000M    1000

#define FXMAC_MSG_PHY_HALF_DUPLEX   0
#define FXMAC_MSG_PHY_FULL_DUPLEX   1

#define MAX_FRAME_SIZE_JUMBO (FXMAC_MSG_MTU_JUMBO + FXMAC_MSG_HDR_SIZE + FXMAC_MSG_TRL_SIZE)

/* Byte alignment of BDs */
#define BD_ALIGNMENT (FXMAC_MSG_DMABD_MINIMUM_ALIGNMENT*2)

/*  frame queue */
#define PQ_QUEUE_SIZE 4096

/*irq priority value*/
#define XMAC_OS_IRQ_PRIORITY_VALUE (configMAX_API_CALL_INTERRUPT_PRIORITY + 1)
FASSERT_STATIC((XMAC_OS_IRQ_PRIORITY_VALUE <= IRQ_PRIORITY_VALUE_15)&&(XMAC_OS_IRQ_PRIORITY_VALUE >= configMAX_API_CALL_INTERRUPT_PRIORITY));

typedef struct 
{
    uintptr data[PQ_QUEUE_SIZE];
    int head, tail, len;
} PqQueue;

typedef enum
{
    FXMAC_MSG_OS_INTERFACE_SGMII = 0,
    FXMAC_MSG_OS_INTERFACE_RMII,
    FXMAC_MSG_OS_INTERFACE_RGMII,
    FXMAC_MSG_OS_INTERFACE_LENGTH
} FXmacMsgFreeRtosInterface;


typedef struct
{
    u8 rx_bdspace[FXMAC_MSG_RX_BDSPACE_LENGTH] __attribute__((aligned(128))); /* 接收bd 缓冲区 */
    u8 tx_bdspace[FXMAC_MSG_TX_BDSPACE_LENGTH] __attribute__((aligned(128))); /* 发送bd 缓冲区 */

    uintptr rx_pbufs_storage[FXMAC_MSG_RX_PBUFS_LENGTH];
    uintptr tx_pbufs_storage[FXMAC_MSG_TX_PBUFS_LENGTH];

} FXmacMsgNetifBuffer;

typedef struct
{
    u32 instance_id;
    FXmacMsgFreeRtosInterface interface;
    u32 autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    u32 phy_speed;  /* FXMAC_PHY_SPEED_XXX */
    u32 phy_duplex; /* FXMAC_PHY_XXX_DUPLEX */
} FXmacMsgPhyControl;

typedef struct
{
    FXmacMsgCtrl instance;
    FXmacMsgPhyControl mac_config;

    FXmacMsgNetifBuffer buffer;

    /* queue to store overflow packets */
    PqQueue recv_q;
    PqQueue send_q;

    /* indicates whether to enbale xmac run in special mode,such as jumbo */
    u32 feature;

    struct LwipPort *stack_pointer; /* Docking data stack data structure */
    u8 hwaddr[FXMAC_MSG_MAX_HARDWARE_ADDRESS_LENGTH];
    void * netif; /* Pointing to the netif */
} FXmacMsgOs;

FXmacMsgOs *FXmacMsgOsGetInstancePointer(FXmacMsgPhyControl *config_p);
FError FXmacMsgOsInit(FXmacMsgOs *instance_p);
void FXmacMsgOsRx(FXmacMsgOs *instance_p, void *pbuf);
FError FXmacMsgOsTx(FXmacMsgOs *instance_p, void *pbuf);
void FXmacMsgOsStop(FXmacMsgOs *instance_p);
void FXmacMsgOsStart(FXmacMsgOs *instance_p);
void FXmacMsgOsRecvHandler(FXmacMsgOs *instance_p);
enum lwip_port_link_status FXmacMsgPhyReconnect(struct LwipPort *xmac_netif_p);

#ifdef __cplusplus
}
#endif

#endif // !
