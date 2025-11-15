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
 * FilePath: e1000e.h
 * Date: 2025-01-03 14:46:52
 * LastEditTime: 2025-01-03 14:46:58
 * Description:  This file is for e1000e driver .Functions in this file are the minimum required functions
 * for this driver.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2025/01/03    first release
 */

#ifndef E1000E_H
#define E1000E_H

#include "ftypes.h"
#include "fassert.h"
#include "ferror_code.h"
#include "fparameters.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FE1000E_SUCCESS              FT_SUCCESS
#define FE1000E_ERR_TIMEOUT          FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 1)
#define FE1000E_ERR_INVALID_DMA_MEM  FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 2)
#define FE1000E_ERR_NOT_READY        FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 3)
#define FE1000E_ERR_TRANS_FAILED     FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 4)
#define FE1000E_ERR_PHY_NOT_SUPPORT  FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 5)
#define FE1000E_ERR_PHY_IS_NOT_FOUND FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 6)
#define FE1000E_ERR_FAILED           FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 7)
#define FE1000E_ERR_PHY_AUTO_FAILED  FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 8)
#define FE1000E_ERR_RECEV_FAILED     FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 9)

/* E1000E */
#define FE1000E_NUM                  1U
#define FE1000E0_ID                  0U

#define ETH_ALEN                     6
#define RX_DESCRIPTORS               128
#define RX_BUFFER_SIZE               2048
#define TX_DESCRIPTORS               128

#define FE1000E_SPEED_10             10U
#define FE1000E_SPEED_100            100U
#define FE1000E_SPEED_1000           1000U
#define FE1000E_SPEED_2500           2500U
#define FE1000E_SPEED_5000           5000U
#define FE1000E_SPEED_10000          10000U
#define FE1000E_SPEED_25000          25000U

#define FE1000E_MTU                  1500U /* max MTU size of Ethernet frame */
#define FE1000E_MTU_JUMBO \
    10240U /* max MTU size of jumbo frame including Ip header + IP payload */
#define FE1000E_HDR_SIZE       14U /* size of Ethernet header  , DA + SA + TYPE*/
#define FE1000E_HDR_VLAN_SIZE  18U /* size of Ethernet header with VLAN */
#define FE1000E_TRL_SIZE       4U  /* size of Ethernet trailer (FCS) */

#define FE1000E_MAX_FRAME_SIZE (FE1000E_MTU + FE1000E_HDR_SIZE + FE1000E_TRL_SIZE)
#define FE1000E_MAX_FRAME_SIZE_JUMBO \
    (FE1000E_MTU_JUMBO + FE1000E_HDR_SIZE + FE1000E_TRL_SIZE)

typedef void (*FE1000EEvtHandler)(void *pCtrl);

/* Legacy TX Descriptor */
struct FE1000ETxDesc
{
    uint64_t addr;
    uint16_t len;
    uint8_t cso;
    uint8_t cmd;
    uint8_t sta;
    uint8_t css;
    uint16_t special;
};

/* Legacy RX Descriptor */
struct FE1000ERxDesc
{
    uint64_t addr;
    uint16_t len;
    uint16_t csum;
    uint8_t sta;
    uint8_t err;
    uint16_t special;
};

typedef enum
{
    FE1000E_PHY_INTERFACE_MODE_SGMII,
    FE1000E_PHY_INTERFACE_MODE_RMII,
    FE1000E_PHY_INTERFACE_MODE_RGMII,
    FE1000E_PHY_INTERFACE_MODE_XGMII,
    FE1000E_PHY_INTERFACE_MODE_USXGMII,
    FE1000E_PHY_INTERFACE_MODE_5GBASER,
    FE1000E_PHY_INTERFACE_MODE_2500BASEX
} FE1000EPhyInterface;

typedef struct
{
    u32 instance_id;
    uintptr base_addr;
    FE1000EPhyInterface interface;
    u32 speed;    /* FE1000E_SPEED_XXX */
    u32 duplex;   /* 1 is full-duplex , 0 is half-duplex */
    u32 auto_neg; /* Enable auto-negotiation - when set active high, autonegotiation operation is enabled. */
} FE1000EConfig;

typedef struct
{
    u32 desc_idx;           /* For Current Desc position */
    u32 desc_buf_idx;       /* For Current Desc buffer buf position */
    u32 desc_first_buf_idx; /* For Identifing the first buffer the frame is in */
    u32 desc_max_num;       /* Max Number for Desc and Desc buffer */
    u8 *desc_buf_base;      /*  Desc buffer Base */
} FE1000ERingDescData;      /* FE1000E DMA描述符表(链式)相关数据 */

enum
{
    FE1000E_TX_COMPLETE_EVT = 0,
    FE1000E_RX_COMPLETE_EVT,
    FE1000E_LINK_STATUS_EVT,
    FE1000E_PHY_STATUS_EVT,
    FE1000E_DMA_ERR_EVT,

    FE1000E_INTR_EVT_NUM
}; /* interrupt event type */

typedef struct
{
    FE1000EConfig config;
    FE1000ERingDescData rx_ring;
    FE1000ERingDescData tx_ring;
    volatile struct FE1000ETxDesc tx[TX_DESCRIPTORS] __aligned(16);
    volatile struct FE1000ERxDesc rx[RX_DESCRIPTORS] __aligned(16);
    uintptr txb[TX_DESCRIPTORS];
    uintptr rxb[RX_DESCRIPTORS];
    uint8_t mac[ETH_ALEN];
    u32 is_ready;
    FE1000EEvtHandler evt_handler[FE1000E_INTR_EVT_NUM]; /* User registered interrupt handler */
    u32 link_status;
} FE1000ECtrl;

typedef enum
{
    FE1000E_LINKDOWN = 0,
    FE1000E_LINKUP = 1,
    FE1000E_NEGOTIATION = 2,
} FE1000ELinkStatus;

FE1000EConfig *FE1000ELookupConfig(u32 instance_id);
FError FE1000ECfgInitialize(FE1000ECtrl *instance_p, FE1000EConfig *cofig_p);
void FE1000ECfgDeInitialize(FE1000ECtrl *pctrl);

void FE1000EStop(FE1000ECtrl *instance_p);
void FE1000EStart(FE1000ECtrl *instance_p);

/* 配置FE1000E的发送DMA描述符和缓冲区 */
FError FE1000ESetupTxDescRing(FE1000ECtrl *instance_p);

/* 配置FE1000E的接收DMA描述符和缓冲区 */
FError FE1000ESetupRxDescRing(FE1000ECtrl *instance_p);

/* 获取FE1000E控制器配置的MAC地址 */
void FE1000EGetMacAddr(FE1000ECtrl *instance_p);

/* 设置FE1000E控制器的MAC地址 */
void FE1000ESetMacAddr(FE1000ECtrl *instance_p);

FError FE1000EReset(FE1000ECtrl *instance_p);

void FE1000EDebugPrint(FE1000ECtrl *instance_p);

FError FE1000ERecvFrame(FE1000ECtrl *instance_p);

FError FE1000ESendFrame(FE1000ECtrl *instance_p, u32 frame_len);

/* 注册FE1000E中断事件处理函数 */
void FE1000ERegisterEvtHandler(FE1000ECtrl *instance_p, u32 event, FE1000EEvtHandler handler);

/* 中断服务函数 */
void FE1000EIrqHandler(s32 vector, void *param);

/* set e1000e irq mask */
void FE1000EIrqEnable(FE1000ECtrl *instance_p, u32 int_mask);

/* disable e1000e irq mask */
void FE1000EIrqDisable(FE1000ECtrl *instance_p, u32 int_mask);

void FE1000EPacketReceptionDone(FE1000ECtrl *instance_p);

#ifdef __cplusplus
}
#endif

#endif // !
