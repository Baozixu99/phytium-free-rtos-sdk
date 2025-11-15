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
 * FilePath: fxmac_msg.h
 * Date: 2024-10-28 14:46:52
 * LastEditTime: 2024-10-28 14:46:52
 * Description:  This file is for xmac msg driver .Functions in this file are the minimum required functions
 * for this driver.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2024/10/28  first release
 */

#ifndef FXMAC_MSG_H
#define FXMAC_MSG_H

#include "ftypes.h"
#include "fassert.h"
#include "ferror_code.h"
#include "fxmac_msg_hw.h"
#include "fxmac_msg_bdring.h"
#include "fparameters.h"
#include "fmsg_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FXMAC_MSG_ERR_INVALID_PARAM FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 0x1u)
#define FXMAC_MSG_ERR_SG_LIST       FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 0x2u)
#define FXMAC_MSG_ERR_GENERAL       FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 0x4u)
#define FXMAC_MSG_ERR_SG_NO_LIST    FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 0x5u)
#define FXMAC_MSG_ERR_PHY_BUSY      FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 0x6u)
#define FXMAC_MSG_PHY_IS_NOT_FOUND  FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 0x7u)
#define FXMAC_MSG_PHY_AUTO_AUTONEGOTIATION_FAILED \
    FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 0x8u)
#define FXMAC_MSG_ERR_MAC_IS_PROCESSING     FT_MAKE_ERRCODE(ErrModBsp, ErrBspEth, 0x9u)

/** @name Configuration options
 *
 * Device configuration options. See the FXMAC_MSG_SetOptions(),
 * FXMAC_MSGClearOptions() and FXMAC_MSG_GetOptions() for information on how to
 * use options.
 *
 * The default state of the options are noted and are what the device and
 * driver will be set to after calling FXMAC_MSG_Reset() or
 * FXMAC_MSG_Initialize().
 *
 * @{
 */

#define FXMAC_MSG_PROMISC_OPTION            0x00000001U
/* Accept all incoming packets.
 *   This option defaults to disabled (cleared) */

#define FXMAC_MSG_FRAME1536_OPTION          0x00000002U
/* Frame larger than 1516 support for Tx & Rx.x
 *   This option defaults to disabled (cleared) */

#define FXMAC_MSG_VLAN_OPTION               0x00000004U
/* VLAN Rx & Tx frame support.
 *   This option defaults to disabled (cleared) */

#define FXMAC_MSG_FLOW_CONTROL_OPTION       0x00000010U
/* Enable recognition of flow control frames on Rx
 *   This option defaults to enabled (set) */

#define FXMAC_MSG_FCS_STRIP_OPTION          0x00000020U
/* Strip FCS and PAD from incoming frames. Note: PAD from VLAN frames is not
 *   stripped.
 *   This option defaults to enabled (set) */

#define FXMAC_MSG_FCS_INSERT_OPTION         0x00000040U
/* Generate FCS field and add PAD automatically for outgoing frames.
 *   This option defaults to disabled (cleared) */

#define FXMAC_MSG_LENTYPE_ERR_OPTION        0x00000080U
/* Enable Length/Type error checking for incoming frames. When this option is
 *   set, the MAC will filter frames that have a mismatched type/length field
 *   and if FXMAC_MSG_REPORT_RXERR_OPTION is set, the user is notified when these
 *   types of frames are encountered. When this option is cleared, the MAC will
 *   allow these types of frames to be received.
 *
 *   This option defaults to disabled (cleared) */

#define FXMAC_MSG_TRANSMITTER_ENABLE_OPTION 0x00000100U
/* Enable the transmitter.
 *   This option defaults to enabled (set) */

#define FXMAC_MSG_RECEIVER_ENABLE_OPTION    0x00000200U
/* Enable the receiver
 *   This option defaults to enabled (set) */

#define FXMAC_MSG_BROADCAST_OPTION          0x00000400U
/* Allow reception of the broadcast address
 *   This option defaults to enabled (set) */

#define FXMAC_MSG_MULTICAST_OPTION          0x00000800U
/* Allows reception of multicast addresses programmed into hash
 *   This option defaults to disabled (clear) */

#define FXMAC_MSG_RX_CHKSUM_ENABLE_OPTION   0x00001000U
/* Enable the RX checksum offload
 *   This option defaults to enabled (set) */

#define FXMAC_MSG_TX_CHKSUM_ENABLE_OPTION   0x00002000U
/* Enable the TX checksum offload
 *   This option defaults to enabled (set) */

#define FXMAC_MSG_JUMBO_ENABLE_OPTION       0x00004000U
#define FXMAC_MSG_SGMII_ENABLE_OPTION       0x00008000U
#define FXMAC_MSG_LOOPBACK_NO_MII_OPTION    0x00010000U
#define FXMAC_MSG_LOOPBACK_USXGMII_OPTION   0x00020000U
#define FXMAC_MSG_UNICAST_OPTION            0x00040000U
#define FXMAC_MSG_TAIL_PTR_OPTION           0x00080000U

#define FXMAC_MSG_GET_TXRING(instance)      (instance.tx_bd_queue.bdring)
#define FXMAC_MSG_GET_RXRING(instance)      (instance.rx_bd_queue.bdring)

#define FXMAC_MSG_DEFAULT_OPTIONS                                                                            \
    ((u32)FXMAC_MSG_FLOW_CONTROL_OPTION | (u32)FXMAC_MSG_FCS_INSERT_OPTION |                                 \
     (u32)FXMAC_MSG_FCS_STRIP_OPTION | (u32)FXMAC_MSG_BROADCAST_OPTION | (u32)FXMAC_MSG_LENTYPE_ERR_OPTION | \
     (u32)FXMAC_MSG_TRANSMITTER_ENABLE_OPTION | (u32)FXMAC_MSG_RECEIVER_ENABLE_OPTION |                      \
     (u32)FXMAC_MSG_RX_CHKSUM_ENABLE_OPTION | (u32)FXMAC_MSG_TX_CHKSUM_ENABLE_OPTION)

typedef enum
{
    FXMAC_MSG_LINKDOWN = 0,
    FXMAC_MSG_LINKUP = 1,
    FXMAC_MSG_NEGOTIATING = 2
} FXmacMsgLinkStatus;

/* The next few constants help upper layers determine the size of memory
 * pools used for Ethernet buffers and descriptor lists.
 */
#define FXMAC_MSG_MAC_ADDR_SIZE 6U /* size of Ethernet header */

#define FXMAC_MSG_MTU           1500U /* max MTU size of Ethernet frame */
#define FXMAC_MSG_MTU_JUMBO \
    10240U /* max MTU size of jumbo frame including Ip header + IP payload */
#define FXMAC_MSG_HDR_SIZE      14U /* size of Ethernet header  , DA + SA + TYPE*/
#define FXMAC_MSG_HDR_VLAN_SIZE 18U /* size of Ethernet header with VLAN */
#define FXMAC_MSG_TRL_SIZE      4U  /* size of Ethernet trailer (FCS) */

#define FXMAC_MSG_MAX_FRAME_SIZE \
    (FXMAC_MSG_MTU + FXMAC_MSG_HDR_SIZE + FXMAC_MSG_TRL_SIZE)
#define FXMAC_MSG_MAX_FRAME_SIZE_JUMBO \
    (FXMAC_MSG_MTU_JUMBO + FXMAC_MSG_HDR_SIZE + FXMAC_MSG_TRL_SIZE)
#define FXMAC_MSG_MAX_VLAN_FRAME_SIZE \
    (FXMAC_MSG_MTU + FXMAC_MSG_HDR_SIZE + FXMAC_MSG_HDR_VLAN_SIZE + FXMAC_MSG_TRL_SIZE)
#define FXMAC_MSG_MAX_VLAN_FRAME_SIZE_JUMBO \
    (FXMAC_MSG_MTU_JUMBO + FXMAC_MSG_HDR_SIZE + FXMAC_MSG_HDR_VLAN_SIZE + FXMAC_MSG_TRL_SIZE)

#define FXMAC_MSG_QUEUE_MAX_NUM      8

/** @name Callback identifiers
 *
 * These constants are used as parameters to FXMAC_MSG_SetHandler()
 * @{
 */
#define FXMAC_MSG_HANDLER_DMASEND    1U /* 发送中断 */
#define FXMAC_MSG_HANDLER_DMARECV    2U /* 接收中断 */
#define FXMAC_MSG_HANDLER_ERROR      3U /* 异常中断 */
#define FXMAC_MSG_HANDLER_LINKCHANGE 4U /* 连接状态 */
#define FXMAC_MSG_HANDLER_RESTART    5U /* 发送描述符队列发生异常 */
/*@}*/

#define FXMAC_MSG_DMA_SG_IS_STARTED  0
#define FXMAC_MSG_DMA_SG_IS_STOPED   1

#define FXMAC_MSG_SPEED_10           10U
#define FXMAC_MSG_SPEED_100          100U
#define FXMAC_MSG_SPEED_1000         1000U
#define FXMAC_MSG_SPEED_2500         2500U
#define FXMAC_MSG_SPEED_5000         5000U
#define FXMAC_MSG_SPEED_10000        10000U
#define FXMAC_MSG_SPEED_25000        25000U

/*  Capability mask bits */
#define FXMAC_MSG_CAPS_ISR_CLEAR_ON_WRITE \
    0x00000001 /* irq status parameters need to be written to clear after they have been read */
// #define FXMAC_MSG_CAPS_TAILPTR              0x00000002 /* use tail ptr */

/** @name Direction identifiers
 *
 *  These are used by several functions and callbacks that need
 *  to specify whether an operation specifies a send or receive channel.
 * @{
 */
#define FXMAC_MSG_SEND 1U /* send direction */
#define FXMAC_MSG_RECV 2U /* receive direction */


typedef void (*FXmacMsgIrqHandler)(void *args);
typedef void (*FXmacMsgErrorIrqHandler)(void *args, u32 direction, u32 error_word);
/* Interface Mode definitions */
/**
 * enum phy_interface_t - Interface Mode definitions
 *
 * @FXMAC_MSG_PHY_INTERFACE_MODE_NA: Not Applicable - don't touch
 * @FXMAC_MSG_PHY_INTERFACE_MODE_INTERNAL: No interface, MAC and FXMAC_MSG_PHY combined
 * @FXMAC_MSG_PHY_INTERFACE_MODE_MII: Media-independent interface
 * @FXMAC_MSG_PHY_INTERFACE_MODE_GMII: Gigabit media-independent interface
 * @FXMAC_MSG_PHY_INTERFACE_MODE_SGMII: Serial gigabit media-independent interface
 * @FXMAC_MSG_PHY_INTERFACE_MODE_TBI: Ten Bit Interface
 * @FXMAC_MSG_PHY_INTERFACE_MODE_REVMII: Reverse Media Independent Interface
 * @FXMAC_MSG_PHY_INTERFACE_MODE_RMII: Reduced Media Independent Interface
 * @FXMAC_MSG_PHY_INTERFACE_MODE_REVRMII: Reduced Media Independent Interface in FXMAC_MSG_PHY role
 * @FXMAC_MSG_PHY_INTERFACE_MODE_RGMII: Reduced gigabit media-independent interface
 * @FXMAC_MSG_PHY_INTERFACE_MODE_RGMII_ID: RGMII with Internal RX+TX delay
 * @FXMAC_MSG_PHY_INTERFACE_MODE_RGMII_RXID: RGMII with Internal RX delay
 * @FXMAC_MSG_PHY_INTERFACE_MODE_RGMII_TXID: RGMII with Internal RX delay
 * @FXMAC_MSG_PHY_INTERFACE_MODE_RTBI: Reduced TBI
 * @FXMAC_MSG_PHY_INTERFACE_MODE_SMII: Serial MII
 * @FXMAC_MSG_PHY_INTERFACE_MODE_XGMII: 10 gigabit media-independent interface
 * @FXMAC_MSG_PHY_INTERFACE_MODE_XLGMII:40 gigabit media-independent interface
 * @FXMAC_MSG_PHY_INTERFACE_MODE_MOCA: Multimedia over Coax
 * @FXMAC_MSG_PHY_INTERFACE_MODE_PSGMII: Penta SGMII
 * @FXMAC_MSG_PHY_INTERFACE_MODE_QSGMII: Quad SGMII
 * @FXMAC_MSG_PHY_INTERFACE_MODE_TRGMII: Turbo RGMII
 * @FXMAC_MSG_PHY_INTERFACE_MODE_100BASEX: 100 BaseX
 * @FXMAC_MSG_PHY_INTERFACE_MODE_1000BASEX: 1000 BaseX
 * @FXMAC_MSG_PHY_INTERFACE_MODE_2500BASEX: 2500 BaseX
 * @FXMAC_MSG_PHY_INTERFACE_MODE_5GBASER: 5G BaseR
 * @FXMAC_MSG_PHY_INTERFACE_MODE_RXAUI: Reduced XAUI
 * @FXMAC_MSG_PHY_INTERFACE_MODE_XAUI: 10 Gigabit Attachment Unit Interface
 * @FXMAC_MSG_PHY_INTERFACE_MODE_10GBASER: 10G BaseR
 * @FXMAC_MSG_PHY_INTERFACE_MODE_25GBASER: 25G BaseR
 * @FXMAC_MSG_PHY_INTERFACE_MODE_USXGMII:  Universal Serial 10GE MII
 * @FXMAC_MSG_PHY_INTERFACE_MODE_10GKR: 10GBASE-KR - with Clause 73 AN
 * @FXMAC_MSG_PHY_INTERFACE_MODE_QUSGMII: Quad Universal SGMII
 * @FXMAC_MSG_PHY_INTERFACE_MODE_1000BASEKX: 1000Base-KX - with Clause 73 AN
 * @FXMAC_MSG_PHY_INTERFACE_MODE_MAX: Book keeping
 *
 * Describes the interface between the MAC and FXMAC_MSG_PHY.
 */
typedef enum
{
    FXMAC_MSG_PHY_INTERFACE_MODE_NA,
    FXMAC_MSG_PHY_INTERFACE_MODE_INTERNAL,
    FXMAC_MSG_PHY_INTERFACE_MODE_MII,
    FXMAC_MSG_PHY_INTERFACE_MODE_GMII,
    FXMAC_MSG_PHY_INTERFACE_MODE_SGMII,
    FXMAC_MSG_PHY_INTERFACE_MODE_TBI,
    FXMAC_MSG_PHY_INTERFACE_MODE_REVMII,
    FXMAC_MSG_PHY_INTERFACE_MODE_RMII,
    FXMAC_MSG_PHY_INTERFACE_MODE_REVRMII,
    FXMAC_MSG_PHY_INTERFACE_MODE_RGMII,
    FXMAC_MSG_PHY_INTERFACE_MODE_RGMII_ID,
    FXMAC_MSG_PHY_INTERFACE_MODE_RGMII_RXID,
    FXMAC_MSG_PHY_INTERFACE_MODE_RGMII_TXID,
    FXMAC_MSG_PHY_INTERFACE_MODE_RTBI,
    FXMAC_MSG_PHY_INTERFACE_MODE_SMII,
    FXMAC_MSG_PHY_INTERFACE_MODE_XGMII,
    FXMAC_MSG_PHY_INTERFACE_MODE_XLGMII,
    FXMAC_MSG_PHY_INTERFACE_MODE_MOCA,
    FXMAC_MSG_PHY_INTERFACE_MODE_PSGMII,
    FXMAC_MSG_PHY_INTERFACE_MODE_QSGMII,
    FXMAC_MSG_PHY_INTERFACE_MODE_TRGMII,
    FXMAC_MSG_PHY_INTERFACE_MODE_100BASEX,
    FXMAC_MSG_PHY_INTERFACE_MODE_1000BASEX,
    FXMAC_MSG_PHY_INTERFACE_MODE_2500BASEX,
    FXMAC_MSG_PHY_INTERFACE_MODE_5GBASER,
    FXMAC_MSG_PHY_INTERFACE_MODE_RXAUI,
    FXMAC_MSG_PHY_INTERFACE_MODE_XAUI,
    /* 10GBASE-R, XFI, SFI - single lane 10G Serdes */
    FXMAC_MSG_PHY_INTERFACE_MODE_10GBASER,
    FXMAC_MSG_PHY_INTERFACE_MODE_25GBASER,
    FXMAC_MSG_PHY_INTERFACE_MODE_USXGMII,
    /* 10GBASE-KR - with Clause 73 AN */
    FXMAC_MSG_PHY_INTERFACE_MODE_10GKR,
    FXMAC_MSG_PHY_INTERFACE_MODE_QUSGMII,
    FXMAC_MSG_PHY_INTERFACE_MODE_1000BASEKX,
    FXMAC_MSG_PHY_INTERFACE_MODE_MAX,
} FXmacMsgPhyInterface;


/* MSG */
typedef struct
{
    u64 tx_octets;
    u64 tx_packets;
    u64 tx_bcast_packets;
    u64 tx_mcase_packets;
    u64 tx_pause_packets;
    u64 tx_64_byte_packets;
    u64 tx_65_127_byte_packets;
    u64 tx_128_255_byte_packets;
    u64 tx_256_511_byte_packets;
    u64 tx_512_1023_byte_packets;
    u64 tx_1024_1518_byte_packets;
    u64 tx_more_than_1518_byte_packets;
    u64 tx_underrun;
    u64 tx_single_collisions;
    u64 tx_multiple_collisions;
    u64 tx_excessive_collisions;
    u64 tx_late_collisions;
    u64 tx_deferred;
    u64 tx_carrier_sense_errors;
    u64 rx_octets;
    u64 rx_packets;
    u64 rx_bcast_packets;
    u64 rx_mcast_packets;
    u64 rx_pause_packets;
    u64 rx_64_byte_packets;
    u64 rx_65_127_byte_packets;
    u64 rx_128_255_byte_packets;
    u64 rx_256_511_byte_packets;
    u64 rx_512_1023_byte_packets;
    u64 rx_1024_1518_byte_packets;
    u64 rx_more_than_1518_byte_packets;
    u64 rx_undersized_packets;
    u64 rx_oversize_packets;
    u64 rx_jabbers;
    u64 rx_fcs_errors;
    u64 rx_length_errors;
    u64 rx_symbol_errors;
    u64 rx_alignment_errors;
    u64 rx_resource_over;
    u64 rx_overruns;
    u64 rx_iphdr_csum_errors;
    u64 rx_tcp_csum_errors;
    u64 rx_udp_csum_errors;
} FXmacMsgStats;


typedef struct
{
    u32 desc0;
    u32 desc1;
    u32 desc2;
    u32 desc3;
} FXmacMsgDmaDesc;

typedef struct
{
    u32 offset;
    u16 regnum;
} __attribute__((packed)) FXmacRegInfo;

typedef struct
{
    int lso;
    int desc_cnt;
    int hdrlen;
    int nocrc;
    u32 mss;
    u32 seq;
} PacketInfo;

typedef struct
{
    u64 addr[4];
    u8 queue_num;
    u8 hw_dma_cap;
} __attribute__((packed)) FXmacMsgRingInfo;

typedef struct
{
    u8 queue_num;
    u8 buffer_size;
} __attribute__((packed)) FXmacMsgRxBufInfo;

typedef struct
{
    u16 dma_burst_length;
    u8 hw_dma_cap;
} __attribute__((packed)) FXmacMsgDmaInfo;

typedef struct
{
    u16 index;
    u16 etype;
} __attribute__((packed)) FXmacMsgEthInfo;

typedef struct
{
    u32 mc_bottom;
    u32 mc_top;
} __attribute__((packed)) FXmacMsgMcInfo;

typedef struct
{
    u32 addrl;
    u16 addrh;
} __attribute__((packed)) FXmacMsgMac;

typedef struct
{
    u8 irq_read_clear;
    u8 dma_data_width;
    u8 dma_addr_width;
    u8 tx_pkt_buffer;
    u8 rx_pkt_buffer;
    u8 pbuf_lso;
    u8 queue_num;
    u8 tx_bd_prefetch;
    u8 rx_bd_prefetch;
    u8 max_rx_fs;
} __attribute__((packed)) FXmacMsgFeature;

typedef struct
{
    u8 interface;
    u8 autoneg;
    u16 duplex;
    u32 speed;
} __attribute__((packed)) FXmacMsgInterfaceInfo;

typedef struct
{
    u32 addr;
    size_t length;
    boolean mapped_as_page;
} FXmacMsgTxSkb;

#define FXMAC_MSG_DATA_LEN 56
typedef struct
{
    u8 reserved; /*取消模块id，作为保留字段，必须为0值*/
    u8 seq;      /*由发起方使用或维护，对端不改变该值，默认为0*/
    u8 cmd_type; /*功能性命令字段，可以用极少位表示，当前保留为8bit*/
    u8 cmd_subid; /*子命令id，与命令实现有关，最多为256个，搭配cmd_type补充*/
    u16 len;    /*在交互数据时，表示payload的长度，交互命令时为0*/
    u8 status1; /*保留字段，各模块根据需要定义，可返回某些函数执行后的返回值*/
    u8 status0; /*协议命令状态，表示命令执行相应*/
    u8 para[FXMAC_MSG_DATA_LEN]; /*传输数据*/
} FXmacMsgInfo;


typedef struct
{
    u32 instance_id; /* Id of device*/
    FDevMsg msg;
    FXmacMsgInfo *xmac_msg;
    volatile uintptr_t extral_mode_base;
    volatile uintptr_t extral_loopback_base;
    FXmacMsgPhyInterface interface;
    u32 speed;    /* FXMAC_MSG_SPEED_XXX */
    u32 duplex;   /* 1 is full-duplex , 0 is half-duplex */
    u32 auto_neg; /* Enable auto-negotiation - when set active high, autonegotiation operation is enabled. */
    u32 pclk_hz;
    u32 max_queue_num; /* Number of Xmac Controller Queues  */
    u32 tx_queue_id;   /* 0 ~ FXMAC_MSG_QUEUE_MAX_NUM ,Index queue number */
    u32 rx_queue_id;   /* 0 ~ FXMAC_MSG_QUEUE_MAX_NUM ,Index queue number */
    u32 hotplug_irq_num;
    u32 dma_brust_length; /*  burst length */
    u32 network_default_config;
    u32 queue_irq_num[FXMAC_MSG_QUEUE_MAX_NUM]; /* mac0 8个 ，其他的 4个 */
    u32 caps;                                   /* used to configure tail ptr feature */
} FXmacMsgConfig;

typedef struct
{
    u32 tx_msg_ring_size;
    u32 rx_msg_ring_size;
    u32 tx_msg_head;
    u32 tx_msg_wr_tail;
    u32 tx_msg_rd_tail;
    u32 rx_msg_head;
    u32 rx_msg_tail;
} FXmacMsg;

typedef struct FXmacMsgCtrl FXmacMsgCtrl;

typedef struct FXmacMsgQueue
{
    FXmacMsgCtrl *pdata;
    int irq;
    int index;

    /* tx queue info */
    unsigned int tx_head;
    unsigned int tx_tail;
    unsigned int tx_xmit_more;
    u64 tx_ring_addr;
    FXmacMsgDmaDesc *tx_ring;
    FXmacMsgTxSkb *tx_skb;

    /* rx queue info */
    unsigned int rx_head;
    unsigned int rx_tail;
    unsigned int rx_next_to_alloc;
    u64 rx_ring_addr;
    FXmacMsgDmaDesc *rx_ring;
} FXmacMsgQueue;

typedef struct FXmacMsgCtrl
{
    u32 is_ready;
    u32 is_started;
    u32 link_status;
    u32 options;
    u32 mask;
    u32 caps;

    FXmacMsgQueueV1 tx_bd_queue;
    FXmacMsgQueueV1 rx_bd_queue;

    //msg
    FXmacMsgConfig config;

    uintptr mac_regs;
    uintptr msg_regs;
    uintptr mhu_regs;

    FXmacMsg msg_ring;

    int queue_irq[FXMAC_MSG_QUEUE_MAX_NUM];
    u32 rx_irq_mask;
    u32 tx_irq_mask;

    u32 rx_ring_size;
    u32 tx_ring_size;

    u32 dma_data_width;
    u32 dma_addr_width;

    int rx_bd_prefetch;
    int tx_bd_prefetch;

    int rx_buffer_len;

    u16 queues_max_num;
    u16 queues_num;
    FXmacMsgQueue queues[FXMAC_MSG_QUEUE_MAX_NUM];
    FXmacMsgStats stats;

    FXmacMsgPhyInterface phy_interface;
    int speed;
    int duplex;
    int autoneg;

    unsigned int max_rx_fs;
    //msg end

    FXmacMsgIrqHandler send_irq_handler;
    void *send_args;

    FXmacMsgIrqHandler recv_irq_handler;
    void *recv_args;

    FXmacMsgErrorIrqHandler error_irq_handler;
    void *error_args;

    FXmacMsgIrqHandler link_change_handler;
    void *link_change_args;

    FXmacMsgIrqHandler restart_handler;
    void *restart_args;

    u32 moudle_id;
    u32 max_mtu_size;
    u32 max_frame_size;
    u32 phy_address;
    u32 rxbuf_mask;
} FXmacMsgCtrl;

typedef enum
{
    FXMAC_MSG_CMD_DEFAULT = 0,
    FXMAC_MSG_CMD_SET,
    FXMAC_MSG_CMD_GET,
    FXMAC_MSG_CMD_DATA,
    FXMAC_MSG_CMD_REPORT,
} FXmacMsgMsgCmdId;

typedef enum
{
    FXMAC_MSG_CMD_DEFAULT_RESERVE = 0,
    FXMAC_MSG_CMD_DEFAULT_RESET_HW,
    FXMAC_MSG_CMD_DEFAULT_RESET_TX_QUEUE,
    FXMAC_MSG_CMD_DEFAULT_RESET_RX_QUEUE,
} FXmacMsgDefaultSubid;

typedef enum
{
    FXMAC_MSG_CMD_SET_INIT_ALL = 0,
    FXMAC_MSG_CMD_SET_INIT_RING = 1,
    FXMAC_MSG_CMD_SET_INIT_TX_RING = 2,
    FXMAC_MSG_CMD_SET_INIT_RX_RING = 3,
    FXMAC_MSG_CMD_SET_INIT_MAC_CONFIG = 4,
    FXMAC_MSG_CMD_SET_ADDR = 5,
    FXMAC_MSG_CMD_SET_DMA_RX_BUFSIZE = 6,
    FXMAC_MSG_CMD_SET_DMA = 7,
    FXMAC_MSG_CMD_SET_CAPS = 8,
    FXMAC_MSG_CMD_SET_TS_CONFIG = 9,
    FXMAC_MSG_CMD_SET_INIT_TX_ENABLE_TRANSMIT = 10,
    FXMAC_MSG_CMD_SET_INIT_RX_ENABLE_RECEIVE = 11,
    FXMAC_MSG_CMD_SET_ENABLE_NETWORK = 12,
    FXMAC_MSG_CMD_SET_DISABLE_NETWORK = 13,
    FXMAC_MSG_CMD_SET_ENABLE_MDIO = 14,
    FXMAC_MSG_CMD_SET_DISABLE_MDIO = 15,
    FXMAC_MSG_CMD_SET_ENABLE_TXCSUM = 16,
    FXMAC_MSG_CMD_SET_DISABLE_TXCSUM = 17,
    FXMAC_MSG_CMD_SET_ENABLE_RXCSUM = 18,
    FXMAC_MSG_CMD_SET_DISABLE_RXCSUM = 19,
    FXMAC_MSG_CMD_SET_ENABLE_PROMISE = 20,
    FXMAC_MSG_CMD_SET_DISABLE_PROMISE = 21,
    FXMAC_MSG_CMD_SET_ENABLE_MC = 22,
    FXMAC_MSG_CMD_SET_DISABLE_MC = 23,
    FXMAC_MSG_CMD_SET_ENABLE_HASH_MC = 24,
    FXMAC_MSG_CMD_SET_ENABLE_PAUSE = 25,
    FXMAC_MSG_CMD_SET_DISABLE_PAUSE = 26,
    FXMAC_MSG_CMD_SET_ENABLE_JUMBO = 27,
    FXMAC_MSG_CMD_SET_DISABLE_JUMBO = 28,
    FXMAC_MSG_CMD_SET_ENABLE_1536_FRAMES = 29,
    FXMAC_MSG_CMD_SET_ENABLE_STRIPCRC = 30,
    FXMAC_MSG_CMD_SET_DISABLE_STRIPCRC = 31,
    FXMAC_MSG_CMD_SET_PCS_LINK_UP = 32,
    FXMAC_MSG_CMD_SET_PCS_LINK_DOWN = 33,
    FXMAC_MSG_CMD_SET_MAC_LINK_CONFIG = 34,
    FXMAC_MSG_CMD_SET_REG_WRITE = 35,
    FXMAC_MSG_CMD_SET_ENABLE_BC = 36,
    FXMAC_MSG_CMD_SET_DISABLE_BC = 37,
    FXMAC_MSG_CMD_SET_ETH_MATCH = 38,
    FXMAC_MSG_CMD_SET_ADD_FDIR = 39,
    FXMAC_MSG_CMD_SET_DEL_FDIR = 40,
    FXMAC_MSG_CMD_SET_ENABLE_AUTONEG = 41,
    FXMAC_MSG_CMD_SET_DISABLE_AUTONEG = 42,
    FXMAC_MSG_CMD_SET_RX_DATA_OFFSET = 43,
    FXMAC_MSG_CMD_SET_WOL = 44,
    FXMAC_MSG_CMD_SET_ENABLE_RSC = 45,
    FXMAC_MSG_CMD_SET_DISABLE_RSC = 46,
    FXMAC_MSG_CMD_SET_ENABLE_TX_START = 47,
    FXMAC_MSG_CMD_SET_ENABLE_PCS_RESET = 48,
    FXMAC_MSG_CMD_SET_DISABLE_PCS_RESET = 49,
    FXMAC_MSG_CMD_SET_MDC = 50,
    FXMAC_MSG_CMD_SET_OUTSTANDING = 51,
} FXmacSetSubid;

typedef enum
{
    FXMAC_MSG_CMD_GET_ADDR,
    FXMAC_MSG_CMD_GET_QUEUENUMS,
    FXMAC_MSG_CMD_GET_CAPS,
    FXMAC_MSG_CMD_GET_BD_PREFETCH,
    FXMAC_MSG_CMD_GET_STATS,
    FXMAC_MSG_CMD_GET_REG_READ,
} FXmacGetSubid;

/* 函数声明 */
/* fxmac_sinit.c */
const FXmacMsgConfig *FXmacMsgLookupConfig(u32 instance_id);

/* fxmac_msg.c */
FError FXmacMsgCfgInitialize(FXmacMsgCtrl *xmac_msg_p, FXmacMsgConfig *config_p);
void FXmacMsgResetHw(FXmacMsgCtrl *pdata);
int FXmacMsgInitHw(FXmacMsgCtrl *instance_p, u8 *mac_address_ptr);
int FXmacMsgInitRingHw(FXmacMsgCtrl *pdata);
int FXmacMsgSetMacAddress(FXmacMsgCtrl *instance, const u8 *addr);
int FXmacMsgGetMacAddress(FXmacMsgCtrl *instance, u8 *addr);
void FXmacMsgMdioIdle(FXmacMsgCtrl *pdata);
int FXmacMsgMdioDataReadC22(FXmacMsgCtrl *pdata, int mii_id, int regnum);
int FXmacMsgMdioDataWriteC22(FXmacMsgCtrl *pdata, int mii_id, int regnum, u16 data);
int FXmacMsgMdioDataReadC45(FXmacMsgCtrl *pdata, int mii_id, int devad, int regnum);
int FXmacMsgMdioDataWriteC45(FXmacMsgCtrl *pdata, int mii_id, int devad, int regnum, u16 data);
int FXmacMsgEnablePromise(FXmacMsgCtrl *pdata, int enable);
int FXmacMsgEnableMulticast(FXmacMsgCtrl *pdata, int enable);
int FXmacMsgSetMcHash(FXmacMsgCtrl *pdata, unsigned int *mc_filter);
int FXmacMsgEnableRxcsum(FXmacMsgCtrl *pdata, int enable);
int FXmacMsgEnableTxcsum(FXmacMsgCtrl *pdata, int enable);
int FXmacMsgEnableMdio(FXmacMsgCtrl *pdata, int enable);
int FXmacMsgEnableAutoneg(FXmacMsgCtrl *pdata, int enable);
int FXmacMsgEnablePause(FXmacMsgCtrl *pdata, int enable);
int FXmacMsgEnableNetwork(FXmacMsgCtrl *pdata, int enable);
void FXmacMsgTxStart(FXmacMsgQueue *queue);
int FXmacMsgGetFeatureAll(FXmacMsgCtrl *pdata);
void FXmacMsgStart(FXmacMsgCtrl *instance_p);
void FXmacMsgStop(FXmacMsgCtrl *instance_p);
void FXmacMsgSetQueuePtr(FXmacMsgCtrl *instance_p, uintptr queue_p, u8 queue_num, u32 direction);

/* fxmac_msg_intr.c */
u32 FXmacMsgGetIrqMask(u32 mask);
u32 FXmacMsgGetIrqStatus(u32 value);
void FXmacMsgEnableIrq(FXmacMsgCtrl *pdata, int queue_index, u32 mask);
void FXmacMsgDisableIrq(FXmacMsgCtrl *pdata, int queue_index, u32 mask);
void FXmacMsgClearIrq(FXmacMsgCtrl *pdata, int queue_index, u32 mask);
unsigned int FXmacMsgGetIrq(FXmacMsgCtrl *pdata, int queue_index);
void FXmacMsgIntrHandler(s32 vector, void *args);
FError FXmacMsgSetHandler(FXmacMsgCtrl *instance_p, u32 handler_type,
                          void *func_pointer, void *call_back_ref);

/* hash table set */
FError FXmacMsgSetHash(FXmacMsgCtrl *intance_p, void *mac_address);
FError FXmacMsgDeleteHash(FXmacMsgCtrl *intance_p, void *mac_address);

#ifdef __cplusplus
}
#endif

#endif // !
