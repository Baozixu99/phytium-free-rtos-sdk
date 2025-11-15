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
 * FilePath: e1000e_lwip_port.h
 * Date: 2025-01-06 14:59:30
 * LastEditTime: 2025-01-06 14:59:30
 * Description:   This file is e1000e portable code for lwip port input,output,status check.
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huangjin     2025/01/06            first release
 */
#ifndef FE1000E_LWIP_PORT_H
#define FE1000E_LWIP_PORT_H

#include "fkernel.h"
#include "ferror_code.h"
#include "e1000e.h"
#include "e1000e_hw.h"
#include "e1000e_phy.h"

#define FREERTOS_E1000E_INIT_ERROR     FT_CODE_ERR(ErrModPort, 0, 0x1)
#define FREERTOS_E1000E_PARAM_ERROR    FT_CODE_ERR(ErrModPort, 0, 0x2)
#define FREERTOS_E1000E_NO_VALID_SPACE FT_CODE_ERR(ErrModPort, 0, 0x3)


#ifdef __cplusplus
extern "C"
{
#endif

#define FE1000E_RX_BDSPACE_LENGTH           0x20000 /* default set 64KB*/
#define FE1000E_TX_BDSPACE_LENGTH           0x20000 /* default set 64KB*/

#define FE1000E_RX_PBUFS_LENGTH             128
#define FE1000E_TX_PBUFS_LENGTH             128

#define FE1000E_RX_DESCRIPTORS              128
#define FE1000E_TX_DESCRIPTORS              128

#define FE1000E_RX_BUFFER_SIZE              2048
#define FE1000E_TX_BUFFER_SIZE              2048


#define FE1000E_MAX_HARDWARE_ADDRESS_LENGTH 6

/* configuration */
#define FE1000E_LWIP_PORT_CONFIG_JUMBO      BIT(0)
#define FE1000E_LWIP_PORT_CONFIG_MULTICAST_ADDRESS_FILITER \
    BIT(1) /* Allow multicast address filtering  */
#define FE1000E_LWIP_PORT_CONFIG_COPY_ALL_FRAMES BIT(2) /* enable copy all frames */
#define FE1000E_LWIP_PORT_CONFIG_CLOSE_FCS_CHECK BIT(3) /* close fcs check */
#define FE1000E_LWIP_PORT_CONFIG_UNICAST_ADDRESS_FILITER \
    BIT(5) /* Allow unicast address filtering  */

/* Phy */
#define FE1000E_PHY_SPEED_10M   10
#define FE1000E_PHY_SPEED_100M  100
#define FE1000E_PHY_SPEED_1000M 1000
#define FE1000E_PHY_SPEED_10G   10000


#define FE1000E_PHY_HALF_DUPLEX 0
#define FE1000E_PHY_FULL_DUPLEX 1

#define FE1000E_RECV_MAX_COUNT  10

/* Byte alignment of BDs */
#define BD_ALIGNMENT            (FE1000E_DMABD_MINIMUM_ALIGNMENT * 2)

/*  frame queue */
#define PQ_QUEUE_SIZE           4096

typedef struct
{
    uintptr data[PQ_QUEUE_SIZE];
    int head, tail, len;
} PqQueue;

typedef enum
{
    FE1000E_LWIP_PORT_INTERFACE_SGMII = 0,
    FE1000E_LWIP_PORT_INTERFACE_RMII,
    FE1000E_LWIP_PORT_INTERFACE_RGMII,
    FE1000E_LWIP_PORT_INTERFACE_USXGMII,
    FE1000E_LWIP_PORT_INTERFACE_LENGTH
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
    u32 phy_speed;       /* FE1000E_PHY_SPEED_XXX */
    u32 phy_duplex;      /* FE1000E_PHY_XXX_DUPLEX */
} FE1000ELwipPortConfig;

typedef struct
{
    FE1000ECtrl instance;
    FE1000ELwipPortConfig e1000e_port_config;

    FE1000ENetifBuffer buffer;

    /* queue to store overflow packets */
    PqQueue recv_q;
    PqQueue send_q;

    /* configuration */
    u32 feature;

    struct LwipPort *stack_pointer; /* Docking data stack data structure */
    u8 hwaddr[FE1000E_MAX_HARDWARE_ADDRESS_LENGTH];
    u32 recv_flg; /* Indicating how many receive interrupts have been triggered */
    volatile u8 ri_is_call; /* Indicates that frame reception is complete, and the receive interrupt function is called */
} FE1000ELwipPort;

FE1000ELwipPort *FE1000ELwipPortGetInstancePointer(u32 FE1000ELwipPortInstanceID);
FError FE1000ELwipPortInit(FE1000ELwipPort *instance_p);
void *FE1000ELwipPortRx(FE1000ELwipPort *instance_p);
FError FE1000ELwipPortTx(FE1000ELwipPort *instance_p, void *tx_buf);
void FE1000ELwipPortStop(FE1000ELwipPort *instance_p);
void FE1000ELwipPortStart(FE1000ELwipPort *instance_p);
void ResetDma(FE1000ELwipPort *instance_p);
enum lwip_port_link_status FE1000EPhyReconnect(struct LwipPort *xmac_netif_p);
enum lwip_port_link_status FE1000EPhyStatus(struct LwipPort *e1000e_netif_p);
void FE1000ERecvHandler(void *arg);
void *FE1000ELwipPortQueueRx(FE1000ELwipPort *instance_p);
int FE1000ELwipPortRxComplete(FE1000ELwipPort *instance_p);
#ifdef __cplusplus
}
#endif

#endif // !
