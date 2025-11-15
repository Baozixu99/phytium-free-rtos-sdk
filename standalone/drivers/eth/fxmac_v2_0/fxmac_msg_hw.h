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
 * FilePath: fxmac_msg_hw.h
 * Date: 2024-10-28 14:46:52
 * LastEditTime: 2024-10-28 14:46:52
 * Description:  This file is hardware definition file. 
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2024/10/28  first release
 */

#ifndef FXMAC_MSG_HW_H
#define FXMAC_MSG_HW_H

#include "fparameters.h"
#include "fio.h"
#include "ftypes.h"
#include "fdrivers_port.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FXMAC_MSG_MAX_MAC_ADDR                     4U
#define FXMAC_MSG_MAX_HASH_BITS                    64U /* Maximum value for hash bits. 2**6 */

#define FXMAC_MSG_SRAM_SIZE                        4096
#define MSG_HDR_LEN                                8
#define READ_REG_NUM_MAX                           16

#define FXMAC_MSG_TX_MSG_HEAD                      0x000
#define FXMAC_MSG_TX_MSG_TAIL                      0x004
#define FXMAC_MSG_RX_MSG_HEAD                      0x008
#define FXMAC_MSG_RX_MSG_TAIL                      0x00c
#define FXMAC_MSG_IMR                              0x020
#define FXMAC_MSG_ISR                              0x02c

#define FXMAC_MSG_SIZE                             0x0048
#define FXMAC_MSG_NETWORK_STATUS                   0x0240
#define FXMAC_MSG_PCS_AN_LP                        0x0244
#define FXMAC_MSG_USX_LINK_STATUS                  0x0248
#define FXMAC_MSG_TIMER_INCR_SUB_NSEC              0x024c
#define FXMAC_MSG_TIMER_INCR                       0x0250
#define FXMAC_MSG_TIMER_MSB_SEC                    0x0254
#define FXMAC_MSG_TIMER_SEC                        0x0258
#define FXMAC_MSG_TIMER_NSEC                       0x025c
#define FXMAC_MSG_TIMER_ADJUST                     0x0260
#define FXMAC_MSG_MDIO                             0x0264
#define FXMAC_MSG(i)                               ((i) * sizeof(FXmacMsgInfo))

#define FXMAC_MSG_FLAGS_MSG_COMP                   0x1
#define FXMAC_MSG_FLAGS_MSG_NOINT                  0x2

#define FXMAC_MSG_TX_PTR(i)                        (0x0100 + ((i)*4))
#define FXMAC_MSG_RX_PTR(i)                        (0x0030 + ((i)*4))
#define FXMAC_MSG_INT_ER(i)                        (0x0140 + ((i)*4))
#define FXMAC_MSG_INT_DR(i)                        (0x0180 + ((i)*4))
#define FXMAC_MSG_INT_MR(i)                        (0x01c0 + ((i)*4))
#define FXMAC_MSG_INT_SR(i)                        (0x0200 + ((i)*4))

/* index */
/* Bitfields in FXMAC_MSG_TX_MSG_TAIL */
#define FXMAC_MSG_TX_MSG_INT_INDEX                 16
#define FXMAC_MSG_TX_MSG_INT_WIDTH                 1

/* Bitfields in FXMAC_MSG_ISR */
#define FXMAC_MSG_COMPLETE_INDEX                   0
#define FXMAC_MSG_COMPLETE_WIDTH                   1

/* Bitfields in FXMAC_MSG_SIZE */
#define FXMAC_MSG_MEM_SIZE_INDEX                   0
#define FXMAC_MSG_MEM_SIZE_WIDTH                   4
#define FXMAC_MSG_TXRING_SIZE_INDEX                8
#define FXMAC_MSG_TXRING_SIZE_WIDTH                6

/* FXMAC_MSG_NETWORK_STATUS */
#define FXMAC_MSG_LINK_INDEX                       0 /* PCS link status */
#define FXMAC_MSG_LINK_WIDTH                       1
#define FXMAC_MSG_MIDLE_INDEX                      2 /* Mdio idle */
#define FXMAC_MSG_MIDLE_WIDTH                      1

/* FXMAC_MSG_PCS_AN_LP */
#define FXMAC_MSG_PCS_LINK_PARTNER_NEXT_PAGE_INDEX 15 /* PCS link status */
#define FXMAC_MSG_PCS_LINK_PARTNER_NEXT_PAGE_WIDTH 1

/* Int stauts/Enable/Disable/Mask Register */
#define FXMAC_MSG_RXCOMP_INDEX                     1 /* Rx complete */
#define FXMAC_MSG_RXCOMP_WIDTH                     1
#define FXMAC_MSG_RUSED_INDEX                      2 /* Rx used bit read */
#define FXMAC_MSG_RUSED_WIDTH                      1
#define FXMAC_MSG_TUSED_INDEX                      3 /* Tx used bit read */
#define FXMAC_MSG_TUSED_WIDTH                      1
#define FXMAC_MSG_TX_UNDER_RUN_INDEX               4 /* Tx under run */
#define FXMAC_MSG_TX_UNDER_RUN_WIDTH               1
#define FXMAC_MSG_RETRY_LIMIT_INDEX                5 /* Retry limit */
#define FXMAC_MSG_RETRY_LIMIT_WIDTH                1
#define FXMAC_MSG_DMA_ERR_INDEX                    6 /* AMBA error */
#define FXMAC_MSG_DMA_ERR_WIDTH                    1
#define FXMAC_MSG_TXCOMP_INDEX                     7 /* Tx complete */
#define FXMAC_MSG_TXCOMP_WIDTH                     1
#define FXMAC_MSG_LINK_CHANGE_INDEX                9 /* Link change */
#define FXMAC_MSG_LINK_CHANGE_WIDTH                1
#define FXMAC_MSG_RXOVERRUN_INDEX                  10 /* Rx overrun */
#define FXMAC_MSG_RXOVERRUN_WIDTH                  1
#define FXMAC_MSG_RESP_ERR_INDEX                   11 /* Resp not ok */
#define FXMAC_MSG_RESP_ERR_WIDTH                   1
#define FXMAC_MSG_IXR_ALL_MASK                     GENMASK(31, 0) /* Everything! */

/* Bitfields in USX_STATUS. */
#define FXMAC_MSG_USX_LINK_INDEX                   0
#define FXMAC_MSG_USX_LINK_WIDTH                   1

/* Mdio read/write Register */
#define FXMAC_MSG_VALUE_INDEX                      0 /* value */
#define FXMAC_MSG_VALUE_WIDTH                      16
#define FXMAC_MSG_CONST_INDEX                      16 /* Must Be 10 */
#define FXMAC_MSG_CONST_WIDTH                      2
#define FXMAC_MSG_REGADDR_INDEX                    18 /* Register address */
#define FXMAC_MSG_REGADDR_WIDTH                    5
#define FXMAC_MSG_PHYADDR_INDEX                    23 /* Phy address */
#define FXMAC_MSG_PHYADDR_WIDTH                    5
#define FXMAC_MSG_MDCOPS_INDEX                     28
#define FXMAC_MSG_MDCOPS_WIDTH                     2
#define FXMAC_MSG_CLAUSESEL_INDEX                  30
#define FXMAC_MSG_CLAUSESEL_WIDTH                  1
#define FXMAC_MSG_C22                              1
#define FXMAC_MSG_C45                              0
#define FXMAC_MSG_C45_ADDR                         0
#define FXMAC_MSG_C45_WRITE                        1
#define FXMAC_MSG_C45_READ                         3
#define FXMAC_MSG_C22_WRITE                        1
#define FXMAC_MSG_C22_READ                         2

#define FXMAC_MSG_BD_ADDR_OFFSET                   0x00000000U /* word 0/addr of BDs */
#define FXMAC_MSG_BD_STAT_OFFSET                   4 /* word 1/status of BDs, 4 bytes */
#define FXMAC_MSG_BD_ADDR_HI_OFFSET                8 /* word 2/addr of BDs */

/* RX描述符状态字 */
/* Word0 */
#define FXMAC_MSG_RXBUF_WRAP_MASK                  BIT(1) /* Wrap bit, last BD */
#define FXMAC_MSG_RXBUF_NEW_MASK                   BIT(0) /* Used bit.. */
#define FXMAC_MSG_RXBUF_ADD_MASK                   GENMASK(31, 2) /* Mask for address */
/* Word1 */
#define FXMAC_MSG_RXBUF_BCAST_MASK                 BIT(31) /* Broadcast frame */
#define FXMAC_MSG_RXBUF_HASH_MASK                  GENMASK(30, 29)
#define FXMAC_MSG_RXBUF_MULTIHASH_MASK             BIT(30) /* Multicast hashed frame */
#define FXMAC_MSG_RXBUF_UNIHASH_MASK               BIT(29) /* Unicast hashed frame */
#define FXMAC_MSG_RXBUF_EXH_MASK                   BIT(27) /* buffer exhausted */
#define FXMAC_MSG_RXBUF_AMATCH_MASK                GENMASK(26, 25) /* Specific address matched */
#define FXMAC_MSG_RXBUF_IDFOUND_MASK               BIT(24)         /* Type ID matched */
#define FXMAC_MSG_RXBUF_IDMATCH_MASK               GENMASK(23, 22) /* ID matched mask */
#define FXMAC_MSG_RXBUF_VLAN_MASK                  BIT(21)         /* VLAN tagged */
#define FXMAC_MSG_RXBUF_PRI_MASK                   BIT(20)         /* Priority tagged */
#define FXMAC_MSG_RXBUF_VPRI_MASK                  GENMASK(19, 17) /* Vlan priority */
#define FXMAC_MSG_RXBUF_CFI_MASK                   BIT(16)         /* CFI frame */
#define FXMAC_MSG_RXBUF_EOF_MASK                   BIT(15)         /* End of frame. */
#define FXMAC_MSG_RXBUF_SOF_MASK                   BIT(14)         /* Start of frame. */
#define FXMAC_MSG_RXBUF_FCS_STATUS_MASK            BIT(13)         /* Status of fcs. */
#define FXMAC_MSG_RXBUF_LEN_MASK                   GENMASK(12, 0) /* Mask for length field */
#define FXMAC_MSG_RXBUF_LEN_JUMBO_MASK             GENMASK(13, 0) /* Mask for jumbo length */

/* TX描述符状态字 */
/* Word0 */
#define FXMAC_MSG_TXBUF_ADDR_MASK                  GENMASK(31, 0) /* Byte address of buffer */
/* Word1 */
#define FXMAC_MSG_TXBUF_USED_MASK                  BIT(31) /* Used bit. */
#define FXMAC_MSG_TXBUF_WRAP_MASK                  BIT(30) /* Wrap bit, last descriptor */
#define FXMAC_MSG_TXBUF_RETRY_MASK                 BIT(29) /* Retry limit exceeded */
#define FXMAC_MSG_TXBUF_URUN_MASK                  BIT(28) /* Transmit underrun occurred */
#define FXMAC_MSG_TXBUF_EXH_MASK                   BIT(27) /* Buffers exhausted */
#define FXMAC_MSG_TXBUF_TCP_MASK                   BIT(26) /* Late collision. */
#define FXMAC_MSG_TXBUF_NOCRC_MASK                 BIT(16) /* No CRC */
#define FXMAC_MSG_TXBUF_LAST_MASK                  BIT(15) /* Last buffer */
#define FXMAC_MSG_TXBUF_LEN_MASK                   GENMASK(13, 0) /* Mask for length field */

/* rx dma desc bit */
/* DMA descriptor bitfields */
#define FXMAC_MSG_RXUSED_INDEX                     0
#define FXMAC_MSG_RXUSED_WIDTH                     1
#define FXMAC_MSG_RXWRAP_INDEX                     1
#define FXMAC_MSG_RXWRAP_WIDTH                     1
#define FXMAC_MSG_RXTSVALID_INDEX                  2
#define FXMAC_MSG_RXTSVALID_WIDTH                  1
#define FXMAC_MSG_RXWADDR_INDEX                    2
#define FXMAC_MSG_RXWADDR_WIDTH                    30

#define FXMAC_MSG_RXFRMLEN_INDEX                   0
#define FXMAC_MSG_RXFRMLEN_WIDTH                   12
#define FXMAC_MSG_RXINDEX_INDEX                    12
#define FXMAC_MSG_RXINDEX_WIDTH                    2
#define FXMAC_MSG_RXSOF_INDEX                      14
#define FXMAC_MSG_RXSOF_WIDTH                      1
#define FXMAC_MSG_RXEOF_INDEX                      15
#define FXMAC_MSG_RXEOF_WIDTH                      1

#define FXMAC_MSG_RXFRMLEN_MASK                    0x1FFF
#define FXMAC_MSG_RXJFRMLEN_MASK                   0x3FFF

#define FXMAC_MSG_RXTYPEID_MATCH_INDEX             22
#define FXMAC_MSG_RXTYPEID_MATCH_WIDTH             2
#define FXMAC_MSG_RXCSUM_INDEX                     22
#define FXMAC_MSG_RXCSUM_WIDTH                     2

/* Buffer descriptor constants */
#define FXMAC_MSG_RXCSUM_NONE                      0
#define FXMAC_MSG_RXCSUM_IP                        1
#define FXMAC_MSG_RXCSUM_IP_TCP                    2
#define FXMAC_MSG_RXCSUM_IP_UDP                    3

#define FXMAC_MSG_TXFRMLEN_INDEX                   0
#define FXMAC_MSG_TXFRMLEN_WIDTH                   14
#define FXMAC_MSG_TXLAST_INDEX                     15
#define FXMAC_MSG_TXLAST_WIDTH                     1
#define FXMAC_MSG_TXNOCRC_INDEX                    16
#define FXMAC_MSG_TXNOCRC_WIDTH                    1
#define FXMAC_MSG_MSSMFS_INDEX                     16
#define FXMAC_MSG_MSSMFS_WIDTH                     14
#define FXMAC_MSG_TXLSO_INDEX                      17
#define FXMAC_MSG_TXLSO_WIDTH                      2
#define FXMAC_MSG_TXTCP_SEQ_SRC_INDEX              19
#define FXMAC_MSG_TXTCP_SEQ_SRC_WIDTH              1
#define FXMAC_MSG_TXTSVALID_INDEX                  23
#define FXMAC_MSG_TXTSVALID_WIDTH                  1
#define FXMAC_MSG_TXWRAP_INDEX                     30
#define FXMAC_MSG_TXWRAP_WIDTH                     1
#define FXMAC_MSG_TXUSED_INDEX                     31
#define FXMAC_MSG_TXUSED_WIDTH                     1

/* dma cpas */
#define HW_DMA_CAP_64B                             0x1
#define HW_DMA_CAP_CSUM                            0x2
#define HW_DMA_CAP_PTP                             0x4
#define HW_DMA_CAP_DDW32                           0x8
#define HW_DMA_CAP_DDW64                           0x10
#define HW_DMA_CAP_DDW128                          0x20

#define FXMAC_MSG_DBW32                            1
#define FXMAC_MSG_DBW64                            2
#define FXMAC_MSG_DBW128                           4

#define FXMAC_MSG_CLK_DIV8                         0
#define FXMAC_MSG_CLK_DIV16                        1
#define FXMAC_MSG_CLK_DIV32                        2
#define FXMAC_MSG_CLK_DIV48                        3
#define FXMAC_MSG_CLK_DIV64                        4
#define FXMAC_MSG_CLK_DIV96                        5
#define FXMAC_MSG_CLK_DIV128                       6
#define FXMAC_MSG_CLK_DIV224                       7

/* macro */
#define FXMAC_MSG_BIT(_field)                      (1 << FXMAC_MSG_##_field##_INDEX)
#define FXMAC_MSG_BITS(_field, value) \
    (((value) & ((1 << FXMAC_MSG_##_field##_WIDTH) - 1)) << FXMAC_MSG_##_field##_INDEX)
#define FXMAC_MSG_GET_BITS(_var, _field) \
    (((_var) >> (FXMAC_MSG_##_field##_INDEX)) & ((0x1 << (FXMAC_MSG_##_field##_WIDTH)) - 1))
#define FXMAC_MSG_SET_BITS(_var, _field, _val)                                             \
    (((_var) & ~(((1 << FXMAC_MSG_##_field##_WIDTH) - 1) << FXMAC_MSG_##_field##_INDEX)) | \
     (((_val) & ((1 << FXMAC_MSG_##_field##_WIDTH) - 1)) << FXMAC_MSG_##_field##_INDEX))

#define FXMAC_MSG_READ(_pdata, _reg) FtIn32((_pdata)->mac_regs + (u32)(_reg))
#define FXMAC_MSG_WRITE(_pdata, _reg, _val) \
    FtOut32((_pdata)->mac_regs + (u32)(_reg), (u32)(_val))
#define FXMAC_MSG_READ_BITS(_pdata, _reg, _field) \
    FXMAC_MSG_GET_BITS(FXMAC_MSG_READ((_pdata), _reg), _field)

/* 中断标志 */
#define FXMAC_MSG_INT_TX_COMPLETE  0x1
#define FXMAC_MSG_INT_TX_ERR       0x2
#define FXMAC_MSG_INT_RX_COMPLETE  0x4
#define FXMAC_MSG_INT_RX_OVERRUN   0x8
#define FXMAC_MSG_INT_RX_DESC_FULL 0x10
#define FXMAC_MSG_INT_LINK         0x20
#define FXMAC_MSG_INT_TX_USED      0x40
#define FXMAC_MSG_DEFUALT_INT_MASK                                                  \
    (FXMAC_MSG_INT_TX_COMPLETE | FXMAC_MSG_INT_TX_ERR | FXMAC_MSG_INT_RX_COMPLETE | \
     FXMAC_MSG_INT_RX_OVERRUN | FXMAC_MSG_INT_RX_DESC_FULL | FXMAC_MSG_INT_LINK)
#define FXMAC_MSG_RX_INT_FLAGS   (FXMAC_MSG_INT_RX_COMPLETE)
#define FXMAC_MSG_TX_INT_FLAGS   (FXMAC_MSG_INT_TX_COMPLETE | FXMAC_MSG_INT_TX_ERR)

#define FXMAC_MSG_MAX_QUEUES     2
#define DEFAULT_DMA_BURST_LENGTH 16
#define DEFAULT_JUMBO_MAX_LENGTH 10240
#define FXMAC_MSG_MAX_TX_LEN     16320
#define FXMAC_MSG_MIN_TX_LEN     64
#define DEFAULT_TX_RING_SIZE     512
#define DEFAULT_RX_RING_SIZE     512
#define MAX_TX_RING_SIZE         1024
#define MAX_RX_RING_SIZE         4096
#define MIN_TX_RING_SIZE         64
#define MIN_RX_RING_SIZE         64
#define DEFAULT_TX_DESC_MIN_FREE 64
#define DEFAULT_RX_DESC_MIN_FREE 64

/* eth类型 */
#define ETH_P_IP                 0x0800
#define ETH_P_IPV6               0x86DD

#define FXMAC_MSG_TX             0x1
#define FXMAC_MSG_RX             0x2

#ifdef __cplusplus
}
#endif

#endif // !
