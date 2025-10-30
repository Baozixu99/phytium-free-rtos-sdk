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
 * FilePath: e1000e_os.h
 * Date: 2025-10-21 16:33:19
 * LastEditTime: 2025-10-21 16:33:19
 * Description:  This file is for e1000e driver.Functions in this file are the minimum required functions for drivers.
 *
 * Modify History:
 *  Ver   Who        Date                   Changes
 * ----- ------    --------     --------------------------------------
 *  1.0  huangjin  2025/10/21            first release
 */

#ifndef FE1000E_OS_H
#define FE1000E_OS_H

#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include "e1000e.h"
#include "fkernel.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FREERTOS_E1000E_INIT_ERROR     FT_CODE_ERR(ErrModPort, 0, 0x1)
#define FREERTOS_E1000E_PARAM_ERROR    FT_CODE_ERR(ErrModPort, 0, 0x2)
#define FREERTOS_E1000E_NO_VALID_SPACE FT_CODE_ERR(ErrModPort, 0, 0x3)

#define FE1000E_RX_BDSPACE_LENGTH    0x20000 /* default set 128KB*/
#define FE1000E_TX_BDSPACE_LENGTH    0x20000 /* default set 128KB*/

#define FE1000E_RX_PBUFS_LENGTH      128
#define FE1000E_TX_PBUFS_LENGTH      128

#define FE1000E_RX_DESCRIPTORS       128
#define FE1000E_TX_DESCRIPTORS       128

#define FE1000E_RX_BUFFER_SIZE       2048
#define FE1000E_TX_BUFFER_SIZE       2048

#define FE1000E_MAX_HARDWARE_ADDRESS_LENGTH 6

#define E1000E_PHY_RESET_ENABLE 1
#define E1000E_PHY_RESET_DISABLE 0

/* configuration */
#define FE1000E_OS_CONFIG_JUMBO  BIT(0)
#define FE1000E_OS_CONFIG_MULTICAST_ADDRESS_FILITER  BIT(1) /* Allow multicast address filtering  */
#define FE1000E_OS_CONFIG_COPY_ALL_FRAMES BIT(2) /* enable copy all frames */
#define FE1000E_OS_CONFIG_CLOSE_FCS_CHECK BIT(3) /* close fcs check */
#define FE1000E_OS_CONFIG_UNICAST_ADDRESS_FILITER BIT(5) /* Allow unicast address filtering  */

/* Phy */
#define FE1000E_PHY_SPEED_10M       10
#define FE1000E_PHY_SPEED_100M      100
#define FE1000E_PHY_SPEED_1000M     1000
#define FE1000E_PHY_SPEED_10G       10000

#define FE1000E_PHY_HALF_DUPLEX   0
#define FE1000E_PHY_FULL_DUPLEX   1

#define MAX_FRAME_SIZE_JUMBO (FE1000E_MTU_JUMBO + FE1000E_HDR_SIZE + FE1000E_TRL_SIZE)

/* Byte alignment of BDs */
#define BD_ALIGNMENT (FE1000E_DMABD_MINIMUM_ALIGNMENT*2)

/*  frame queue */
#define PQ_QUEUE_SIZE 4096

/*irq priority value*/
#define E1000E_OS_IRQ_PRIORITY_VALUE (configMAX_API_CALL_INTERRUPT_PRIORITY+1)
FASSERT_STATIC((E1000E_OS_IRQ_PRIORITY_VALUE <= IRQ_PRIORITY_VALUE_15)&&(E1000E_OS_IRQ_PRIORITY_VALUE >= configMAX_API_CALL_INTERRUPT_PRIORITY));

typedef struct 
{
    uintptr data[PQ_QUEUE_SIZE];
    int head, tail, len;
} PqQueue;

typedef enum
{
    FE1000E_OS_INTERFACE_SGMII = 0,
    FE1000E_OS_INTERFACE_RMII,
    FE1000E_OS_INTERFACE_RGMII,
    FE1000E_OS_INTERFACE_USXGMII,
    FE1000E_OS_INTERFACE_LENGTH
} FE1000ELwipPortInterface;


typedef struct
{
    volatile struct FE1000ETxDesc tx[FE1000E_TX_DESCRIPTORS] __aligned(16);
    volatile struct FE1000ERxDesc rx[FE1000E_RX_DESCRIPTORS] __aligned(16);

    uint8_t txb[FE1000E_TX_DESCRIPTORS * FE1000E_TX_BUFFER_SIZE];
    uint8_t rxb[FE1000E_RX_DESCRIPTORS * FE1000E_RX_BUFFER_SIZE];
} FE1000ENetifBuffer;

typedef struct
{
    u32 instance_id;
    FE1000ELwipPortInterface interface;
    u32 autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    u32 phy_speed;  /* FE1000E_PHY_SPEED_XXX */
    u32 phy_duplex; /* FE1000E_PHY_XXX_DUPLEX */
} FE1000ELwipPortConfig;

typedef struct
{
    FE1000ECtrl instance;
    FE1000ELwipPortConfig e1000e_port_config;

    FE1000ENetifBuffer buffer;

    /* queue to store overflow packets */
    PqQueue recv_q;
    PqQueue send_q;

    /* indicates whether to enbale e1000e run in special mode,such as jumbo */
    u32 feature;

    struct LwipPort *stack_pointer; /* Docking data stack data structure */
    u8 hwaddr[FE1000E_MAX_HARDWARE_ADDRESS_LENGTH];
    void * netif; /* Pointing to the netif */
} FE1000EOs;

FE1000EOs *FE1000ELwipPortGetInstancePointer(u32 FE1000ELwipPortInstanceID);
FError FE1000ELwipPortInit(FE1000EOs *instance_p);
void *FE1000ELwipPortRx(FE1000EOs *instance_p);
FError FE1000ELwipPortTx(FE1000EOs *instance_p, void *tx_buf);
void FE1000ELwipPortStop(FE1000EOs *instance_p);
void FE1000ELwipPortStart(FE1000EOs *instance_p);
void ResetDma(FE1000EOs *instance_p);
enum lwip_port_link_status FE1000EPhyReconnect(struct LwipPort *xmac_netif_p);
enum lwip_port_link_status FE1000EPhyStatus(struct LwipPort *e1000e_netif_p);
void FE1000ERecvHandler(void *arg);
void *FE1000ELwipPortQueueRx(FE1000EOs *instance_p);
int FE1000ELwipPortRxComplete(FE1000EOs *instance_p);

#ifdef __cplusplus
}
#endif

#endif // !
