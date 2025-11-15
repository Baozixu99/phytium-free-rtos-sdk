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
 * FilePath: e1000e_hw.h
 * Date: 2025-01-03 14:46:52
 * LastEditTime: 2025-01-03 14:46:58
 * Description:  This file is hardware definition file. 
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2025/01/03    first release
 */

#ifndef E1000E_HW_H
#define E1000E_HW_H

#include "fparameters.h"
#include "fio.h"
#include "ftypes.h"
#include "fdrivers_port.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define E1000_CTRL                         0x00000   /* Device Control - RW */
#define CTRL_FD                            (1)       /* Full-Duplex */
#define CTRL_LRST                          (1 << 3)  /* Link Reset */
#define CTRL_SLU                           (1 << 6)  /* Set Link Up */
#define CTRL_FRCSPD                        (1 << 11) /* Force Speed */
#define CTRL_FRCDPLX                       (1 << 12) /* Force Duplex */
#define CTRL_RST                           (1 << 26) /* Device Reset */

#define E1000_STATUS                       0x00008 /* Device Status - RO */
#define STATUS_LU                          (1 << 1)
#define STATUS_LU_SHFT                     1
#define STATUS_SPEEDSHFT                   6
#define STATUS_SPEED                       (3 << STATUS_SPEEDSHFT)

#define E1000_EECD                         0x00010 /* EEPROM/Flash Control - RW */
#define E1000_EERD                         0x00014 /* EEPROM Read - RW */
#define EERD_START                         (1 << 0)
#define EERD_DONE                          (1 << 1)
#define EERD_ADDR_SHIFT                    2
#define EERD_DATA_SHIFT                    16

#define E1000_CTRL_EXT                     0x00018 /* Extended Device Control - RW */
#define E1000_FLA                          0x0001C /* Flash Access - RW */
#define E1000_MDIC                         0x00020 /* MDI Control - RW */
#define MDIC_REGADD_SHFT                   16
#define MDIC_PHYADD                        (0x1 << 21)
#define MDIC_OP_WRITE                      (0x1 << 26)
#define MDIC_OP_READ                       (0x2 << 26)
#define MDIC_READY                         (0x1 << 28)
#define PHY_CTRL                           0
#define PHYC_POWER_DOWN                    (1 << 11)

#define E1000_FCAL                         0x00028 /* Flow Control Address Low - RW */
#define E1000_FCAH                         0x0002C /* Flow Control Address High -RW */
#define E1000_FEXT                         0x0002C /* Future Extended - RW */
#define E1000_FEXTNVM                      0x00028 /* Future Extended NVM - RW */
#define E1000_FEXTNVM3                     0x0003C /* Future Extended NVM 3 - RW */
#define E1000_FEXTNVM4                     0x00024 /* Future Extended NVM 4 - RW */
#define E1000_FEXTNVM5                     0x00014 /* Future Extended NVM 5 - RW */
#define E1000_FEXTNVM6                     0x00010 /* Future Extended NVM 6 - RW */
#define E1000_FEXTNVM7                     0x000E4 /* Future Extended NVM 7 - RW */
#define E1000_FEXTNVM8                     0x5BB0  /* Future Extended NVM 8 - RW */
#define E1000_FEXTNVM9                     0x5BB4  /* Future Extended NVM 9 - RW */
#define E1000_FEXTNVM11                    0x5BBC  /* Future Extended NVM 11 - RW */
#define E1000_FEXTNVM12                    0x5BC0  /* Future Extended NVM 12 - RW */
#define E1000_PCIEANACFG                   0x00F18 /* PCIE Analog Config */
#define E1000_DPGFR                        0x00FAC /* Dynamic Power Gate Force Control Register */

#define E1000_FCT                          0x00030 /* Flow Control Type - RW */
#define E1000_VET                          0x00038 /* VLAN Ether Type - RW */
#define E1000_ICR                          0x000C0 /* Interrupt Cause Read - R/clr */
#define ICR_TXDW                           (1) /* Transmit Descriptor Written Back */
#define ICR_TXQE                           (1 << 1) /* Transmit Queue Empty */
#define ICR_LSC                            (1 << 2) /* Link Status Change */
#define ICR_RXDMT0                         (1 << 4) /* Receive Descriptor Minimum Threshold hit */
#define ICR_RXO                            (1 << 6) /* Receiver Overrun */
#define ICR_RXT0                           (1 << 7) /* Receiver Timer Interrupt 0 */

#define E1000_ITR                          0x000C4 /* Interrupt Throttling Rate - RW */
#define E1000_ICS                          0x000C8 /* Interrupt Cause Set - WO */
#define ICS_TXDW                           (1) /* Transmit Descriptor Written Back */
#define ICS_TXQE                           (1 << 1) /* Transmit Queue Empty */
#define ICS_LSC                            (1 << 2) /* Link Status Change */
#define ICS_RXDMT0                         (1 << 4) /* Receive Descriptor Minimum Threshold hit */
#define ICS_RXO                            (1 << 6) /* Receiver FIFO Overrun */
#define ICS_RXT0                           (1 << 7) /* Receiver Timer Interrupt 0 */

#define E1000_IMS                          0x000D0 /* Interrupt Mask Set - RW */
#define IMS_TXDW                           (1) /* Transmit Descriptor Written Back */
#define IMS_TXQE                           (1 << 1) /* Transmit Queue Empty */
#define IMS_LSC                            (1 << 2) /* Link Status Change */
#define IMS_RXDMT0                         (1 << 4) /* Receive Descriptor Minimum Threshold hit */
#define IMS_RXO                            (1 << 6)  /* Receiver FIFO Overrun */
#define IMS_RXT0                           (1 << 7)  /* Receiver Timer Interrupt 0 */
#define IMS_RXQ0                           (1 << 20) /* Rx Queue 0 Interrupt */
#define IMS_TXQ0                           (1 << 22) /* Tx Queue 0 Interrupt */
#define IMS_ALL_MASK                       GENMASK(31, 0)

#define E1000_IMC                          0x000D8 /* Interrupt Mask Clear - WO */
#define E1000_IAM                          0x000E0 /* Interrupt Acknowledge Auto Mask */
#define E1000_IVAR                         0x000E4 /* Interrupt Vector Allocation Register - RW */
#define E1000_SVCR                         0x000F0
#define E1000_SVT                          0x000F4
#define E1000_LPIC                         0x000FC /* Low Power IDLE control */

#define E1000_RCTL                         0x00100 /* Rx Control - RW */
#define RCTL_EN                            (1 << 1)
#define RCTL_UPE                           (1 << 3) /* Unicast Promiscuous Enabled */
#define RCTL_MPE                           (1 << 4) /* Multicast Promiscuous Enabled */
#define RCTL_RDMTS_1_2                     (0 << 8)
#define RCTL_RDMTS_1_4                     (1 << 8)
#define RCTL_RDMTS_1_8                     (1 << 9)
#define RCTL_BAM                           (1 << 15) /* Broadcast Accept Mode */
#define RCTL_BSIZE_2048                    (0 << 16)
#define RCTL_SECRC                         (1 << 26)

#define E1000_FCTTV                        0x00170 /* Flow Control Transmit Timer Value - RW */
#define E1000_TXCW                         0x00178 /* Tx Configuration Word - RW */
#define E1000_RXCW                         0x00180 /* Rx Configuration Word - RO */
#define E1000_PBA_ECC                      0x01100 /* PBA ECC Register */

#define E1000_TCTL                         0x00400 /* Tx Control - RW */
#define TCTL_EN                            (1 << 1)
#define TCTL_PSP                           (1 << 3)
#define TCTL_CT_DEF                        (0x0F << 4)
#define TCTL_COLD_DEF                      (0x3F << 12)

#define E1000_TCTL_EXT                     0x00404 /* Extended Tx Control - RW */

#define E1000_TIPG                         0x00410 /* Tx Inter-packet gap -RW */
#define TIPG_IPGT_DEF                      (10 << 0)
#define TIPG_IPGR1_DEF                     (10 << 10)
#define TIPG_IPGR2_DEF                     (10 << 20)

#define E1000_AIT                          0x00458 /* Adaptive Interframe Spacing Throttle - RW */
#define E1000_LEDCTL                       0x00E00 /* LED Control - RW */
#define E1000_EXTCNF_CTRL                  0x00F00 /* Extended Configuration Control */
#define E1000_EXTCNF_SIZE                  0x00F08 /* Extended Configuration Size */
#define E1000_PBA                          0x01000 /* Packet Buffer Allocation - RW */
#define E1000_PBS                          0x01008 /* Packet Buffer Size */
#define E1000_PBECCSTS                     0x0100C /* Packet Buffer ECC Status - RW */
#define E1000_IOSFPC                       0x00F28 /* TX corrupted data  */
#define E1000_EEMNGCTL                     0x01010 /* MNG EEprom Control */
#define E1000_EEWR                         0x0102C /* EEPROM Write Register - RW */
#define E1000_FLOP                         0x0103C /* FLASH Opcode Register */
#define E1000_ERT                          0x02008 /* Early Rx Threshold - RW */
#define E1000_FCRTL                        0x02160 /* Flow Control Receive Threshold Low - RW */
#define E1000_FCRTH                        0x02168 /* Flow Control Receive Threshold High - RW */
#define E1000_PSRCTL                       0x02170 /* Packet Split Receive Control - RW */
#define E1000_RDFH                         0x02410 /* Rx Data FIFO Head - RW */
#define E1000_RDFT                         0x02418 /* Rx Data FIFO Tail - RW */
#define E1000_RDFHS                        0x02420 /* Rx Data FIFO Head Saved - RW */
#define E1000_RDFTS                        0x02428 /* Rx Data FIFO Tail Saved - RW */
#define E1000_RDFPC                        0x02430 /* Rx Data FIFO Packet Count - RW */
#define E1000_RDTR                         0x02820 /* Rx Delay Timer - RW */
#define E1000_RADV                         0x0282C /* Rx Interrupt Absolute Delay Timer - RW */

#define E1000_RDBAL                        0x02800 /* RX Descriptor Base Address Low - RW */
#define E1000_RDBAH                        0x02804 /* RX Descriptor Base Address High - RW */
#define E1000_RDLEN                        0x02808 /* RX Descriptor Length - RW */
#define E1000_RDH                          0x02810 /* RX Descriptor Head - RW */
#define E1000_RDT                          0x02818 /* RX Descriptor Tail - RW */
#define E1000_RDTR                         0x02820 /* RX Delay Timer - RW */

#define E1000_RXDCTL                       0x02828 /* RX Descriptor Control queue 0 - RW */
#define RXDCTL_WTHRESH                     (1 << 16) /* Write Back Threshold */
#define RXDCTL_GRAN                        (1 << 24) /* Granularity */
#define RXDCTL_ENABLE                      (1 << 25)

#define E1000_TDBAL                        0x03800 /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH                        0x03804 /* TX Descriptor Base Address High - RW */
#define E1000_TDLEN                        0x03808 /* TX Descriptor Length - RW */
#define E1000_TDH                          0x03810 /* TX Descriptor Head - RW */
#define E1000_TDT                          0x03818 /* TX Descripotr Tail - RW */
#define E1000_TIDV                         0x03820 /* TX Interrupt Delay Value - RW */

#define E1000_TXDCTL                       0x03828   /* TX Descriptor Control - RW */
#define TXDCTL_WTHRESH                     (1 << 16) /* Write Back Threshold */
#define TXDCTL_82571                       (1 << 22)
#define TXDCTL_GRAN                        (1 << 24) /* Granularity */
#define TXDCTL_ENABLE                      (1 << 25)

#define E1000_TARC0                        0x03840 /* TX Arbitration Count (0) */
#define E1000_RAL                          0x05400 /* Receive Address Low - RW Array */
#define E1000_RAH                          0x05404 /* Receive Address High - RW Array */
#define RAH_AV                             (1 << 31)

#define E1000_MRQC                         0x05818 /* Multiple Receive Control - RW */
#define E1000_GCR                          0x05B00 /* PCI-Ex Control */
#define E1000_GSCL_1                       0x05B10 /* PCI-Ex Statistic Control #1 */
#define E1000_GSCL_2                       0x05B14 /* PCI-Ex Statistic Control #2 */
#define E1000_GSCL_3                       0x05B18 /* PCI-Ex Statistic Control #3 */
#define E1000_GSCL_4                       0x05B1C /* PCI-Ex Statistic Control #4 */
#define E1000_FACTPS                       0x05B30 /* Function Active and Power State to MNG */
#define E1000_SWSM                         0x05B50 /* SW Semaphore */
#define E1000_FWSM                         0x05B54 /* FW Semaphore */
#define E1000_FFLT_DBG                     0x05F04 /* Debug Register */
#define E1000_HICR                         0x08F00 /* Host Interface Control */

/* 描述符 */
#define RDESC_STA_DD                       (1) /* Descriptor Done */
#define TDESC_STA_DD                       (1) /* Descriptor Done */

#define TDESC_EOP                          (1)      /* End Of Packet */
#define TDESC_IFCS                         (1 << 1) /* Insert FCS */
#define TDESC_RS                           (1 << 3) /* Report Status */


#define FE1000E_READREG32(add, reg_offset) FtIn32(add + (u32)reg_offset)
#define FE1000E_WRITEREG32(add, reg_offset, reg_value) \
    FtOut32(add + (u32)reg_offset, (u32)reg_value)
#define FE1000E_SETBIT32(add, reg_offset, reg_value) \
    FtSetBit32(add + (u32)reg_offset, (u32)reg_value)
#define FE1000E_CLEARBIT32(add, reg_offset, reg_value) \
    FtClearBit32(add + (u32)reg_offset, (u32)reg_value)

/* 触发FE1000E控制器软件复位 */
FError FE1000ESoftwareReset(uintptr addr);

#ifdef __cplusplus
}
#endif

#endif // !
