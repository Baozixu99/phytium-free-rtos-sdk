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
 * FilePath: fi2c_msg_hw.h
 * Date: 2024-10-1 14:53:42
 * LastEditTime: 2024-10-10 08:36:14
 * Description:  This file is for I2C register definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangzq      2024/10/10 first commit
 */
#ifndef FI2C_MSG_HW_H
#define FI2C_MSG_HW_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "fio.h"
#include "ferror_code.h"
#include "fmsg_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Type Definitions **********/

/***************************** Macro Definitions **********/
#define FI2C_MSG_READ_REG32(addr, reg_offset) FtIn32((addr) + (u32)(reg_offset))
#define FI2C_MSG_WRITE_REG32(addr, reg_offset, reg_value) \
    FtOut32((addr) + (u32)reg_offset, (u32)reg_value)

#define FI2C_IO_SLAVE_RX_INFO_SIZE           2
#define FI2C_IO_SLAVE_EVENT_MAX_CNT          64
/***************************** Constant Definitions **********/
#define FI2C_M_RD                            0x0001 /* read data, from slave to master */
#define FI2C_M_TEN                           0x0010 /* this is a ten bit chip address */
#define FI2C_M_RECV_LEN                      0x0400 /* length will be first received byte */
#define FI2C_M_NO_RD_ACK                     0x0800 /* if I2C_FUNC_PROTOCOL_MANGLING */
#define FI2C_M_IGNORE_NAK                    0x1000 /* if I2C_FUNC_PROTOCOL_MANGLING */
#define FI2C_M_REV_DIR_ADDR                  0x2000 /* if I2C_FUNC_PROTOCOL_MANGLING */
#define FI2C_M_NOSTART                       0x4000 /* if I2C_FUNC_NOSTART */
#define FI2C_M_STOP                          0x8000 /* if I2C_FUNC_PROTOCOL_MANGLING */
#define FI2C_M_DMA_SAFE                      0x0002 /* if I2C_FUNC_DMA_SAFE */
#define FI2C_M_RD                            0x0001 /* read data, from slave to master */
#define FI2C_M_TEN                           0x0010 /* this is a ten bit chip address */
#define FI2C_M_RECV_LEN                      0x0400 /* length will be first received byte */
#define FI2C_M_NO_RD_ACK                     0x0800 /* if I2C_FUNC_PROTOCOL_MANGLING */
#define FI2C_M_IGNORE_NAK                    0x1000 /* if I2C_FUNC_PROTOCOL_MANGLING */
#define FI2C_M_REV_DIR_ADDR                  0x2000 /* if I2C_FUNC_PROTOCOL_MANGLING */
#define FI2C_M_NOSTART                       0x4000 /* if I2C_FUNC_NOSTART */
#define FI2C_M_STOP                          0x8000 /* if I2C_FUNC_PROTOCOL_MANGLING */

#define FI2C_MSG_FUNC_I2C                    0x00000001
#define FI2C_MSG_FUNC_10BIT_ADDR             0x00000002
#define FI2C_MSG_FUNC_PROTOCOL_MANGLING      0x00000004
#define FI2C_MSG_FUNC_SMBUS_PEC              0x00000008
#define FI2C_MSG_FUNC_NOSTART                0x000000010
#define FI2C_MSG_FUNC_SLAVE                  0x000000020
#define FI2C_MSG_FUNC_SMBUS_BLOCK_PROC_CALL  0x00008000
#define FI2C_MSG_FUNC_SMBUS_QUICK            0x00010000
#define FI2C_MSG_FUNC_SMBUS_READ_BYTE        0x00020000
#define FI2C_MSG_FUNC_SMBUS_WRITE_BYTE       0x00040000
#define FI2C_MSG_FUNC_SMBUS_READ_BYTE_DATA   0x00080000
#define FI2C_MSG_FUNC_SMBUS_WRITE_BYTE_DATA  0x00100000
#define FI2C_MSG_FUNC_SMBUS_READ_WORD_DATA   0x00200000
#define FI2C_MSG_FUNC_SMBUS_WRITE_WORD_DATA  0x00400000
#define FI2C_MSG_FUNC_SMBUS_PROC_CALL        0x00800000
#define FI2C_MSG_FUNC_SMBUS_READ_BLOCK_DATA  0x01000000
#define FI2C_MSG_FUNC_SMBUS_WRITE_BLOCK_DATA 0x02000000
#define FI2C_MSG_FUNC_SMBUS_READ_I2C_BLOCK   0x04000000
#define FI2C_MSG_FUNC_SMBUS_WRITE_I2C_BLOCK  0x08000000
#define FI2C_MSG_FUNC_SMBUS_HOST_NOTIFY      0x10000000

#define FI2C_FUNC_SMBUS_BYTE                 (FI2C_FUNC_SMBUS_READ_BYTE | FI2C_FUNC_SMBUS_WRITE_BYTE)
#define FI2C_FUNC_SMBUS_WORD_DATA \
    (FI2C_FUNC_SMBUS_READ_WORD_DATA | FI2C_FUNC_SMBUS_WRITE_WORD_DATA)
#define FI2C_FUNC_SMBUS_BLOCK_DATA \
    (FI2C_FUNC_SMBUS_READ_BLOCK_DATA | FI2C_FUNC_SMBUS_WRITE_BLOCK_DATA)
#define FI2C_FUNC_SMBUS_I2C_BLOCK \
    (FI2C_FUNC_SMBUS_READ_I2C_BLOCK | FI2C_FUNC_SMBUS_WRITE_I2C_BLOCK)

#define FI2C_MSG_UNIT_SIZE                  10
#define FI2C_MSG_DATA_RESV_LEN              2
#define FI2C_MSG_SHMEM_RX_ADDR_OFFSET       320
#define FI2C_MSG_CMDDATA_SIZE               64

#define FI2C_MSG_IN_INTERRUPT_MODE          0
#define FI2C_MSG_ENABLE_INTERRUPT           1

#define FI2C_MSG_REGFILE_HEARTBIT_VAL       BIT(2)
#define FI2C_MSG_LOG_SIZE_LOW_SHIFT         4
#define FI2C_MSG_LOG_SIZE_MASK              GENMASK(7, 4)
#define FI2C_MSG_LOG_ADDR_SHIFT             10
#define FI2C_MSG_LOG_ADDR_LOW_SHIFT         8
#define FI2C_MSG_LOG_ADDR_MASK              GENMASK(29, 8)
#define FI2C_MSG_LOG_ADDR_LOCK_VAL          BIT(31)
#define FI2C_MSG_LOG_ADDR_LOG_FLAG          BIT(3)
#define FI2C_MSG_LOG_ADDR_LOG_ALIVE         BIT(1)
#define FI2C_MSG_LOG_ADDR_LOG_DEBUG         BIT(0)
#define FI2C_MSG_REGFILE_DISABLE_INTR_VAL   GENMASK(31, 0)
#define FI2C_MSG_REGFILE_ENABLE_INTR_VAL    0
#define FI2C_MSG_REGFILE_M2S_SET_INTR_VAL   BIT(4)

#define FI2C_MSG_S2M_INTR_4                 BIT(4)
#define FI2C_MSG_ADAPTER_MODULE_ON          1
#define FI2C_MSG_ADAPTER_MODULE_OFF         2
#define FI2C_MSG_ADAPTER_MODULE_RESET       3

#define FI2C_MSG_BUS_SPEED_STANARD_MODE     1
#define FI2C_MSG_BUS_SPEED_FAST_MODE        2
#define FI2C_MSG_BUS_SPEED_HIGH_MODE        3
#define FI2C_MSG_BUS_SPEED_TRANS_PARAM_MODE 0
#define FI2C_MSG_BUS_SPEED_CALC_MODE        1

#define FI2C_MSG_DEFAULT_FUNCTIONALITY                                        \
    (FI2C_MSG_FUNC_I2C | FI2C_MSG_FUNC_SMBUS_QUICK | FI2C_FUNC_SMBUS_BYTE | | \
     FI2C_FUNC_SMBUS_WORD_DATA | FI2C_FUNC_SMBUS_BLOCK_DATA | FI2C_FUNC_SMBUS_I2C_BLOCK)

#define FI2C_MSG_VIRT_Slave_SHMEM_TX_MSG_MAX_CNT 1
#define FI2C_MSG_MASTER_MODE_FLAG                1
#define FI2C_MSG_SLAVE_MODE_FLAG                 0
#define FI2C_MSG_RESTART_FLAG                    1
#define FI2C_MSG_NOT_RESTART_FLAG                0
#define FI2C_MSG_SLAVE_DATA_IN                   0
#define FI2C_MSG_SLAVE_DATA_OUT                  1

#define FI2C_MSG_SPEED_100K                      100000
#define FI2C_MSG_SPEED_400K                      400000
#define FI2C_MSG_SPEED_1000K                     1000000
#define FI2C_MSG_SPEED_3400K                     3400000

#define FI2C_MSG_CON_MASTER                      0x1
#define FI2C_MSG_CON_SPEED_STD                   0x2
#define FI2C_MSG_CON_SPEED_FAST                  0x4
#define FI2C_MSG_CON_SPEED_HIGH                  0x6
#define FI2C_MSG_CON_MASK                        0x6
#define FI2C_MSG_CON_10BIT_ADDR_SLAVE            0x8
#define FI2C_MSG_CON_10BIT_ADDR_MASTER           0x10
#define FI2C_MSG_CON_RESTART_EN                  0x20
#define FI2C_MSG_SLAVE_DISABLE                   0x40
#define FI2C_MSG_STOP_DET_IFADDRESSED            0x80
#define FI2C_MSG_STOP_DET_IFADDR_MASK            0x80
#define FI2C_MSG_CON_TX_EMPTY_CTRL               0x100
#define FI2C_MSG_CON_RX_FIFO_FULL_HLD_CTRL       0x200
#define FI2C_MSG_CON_RX_FIFO_FULL_HLD_MASK       0x200

#define FT_I2C_REGFILE_HEARTBIT_VAL              BIT(2)
#define FT_I2C_LOG_ADDR_LOG_ALIVE                BIT(1)

#define FI2C_MSG_CON                             0x0
#define FI2C_MSG_TAR                             0x4
#define FI2C_MSG_SAR                             0x8
#define FI2C_MSG_DATA_CMD                        0x10
#define FI2C_MSG_SS_SCL_HCNT                     0x14
#define FI2C_MSG_SS_SCL_LCNT                     0x18
#define FI2C_MSG_FS_SCL_HCNT                     0x1C
#define FI2C_MSG_FS_SCL_LCNT                     0x20
#define FI2C_MSG_HS_SCL_HCNT                     0x24
#define FI2C_MSG_HS_SCL_LCNT                     0x28
#define FI2C_MSG_INTR_STAT                       0x2C
#define FI2C_MSG_INTR_MASK                       0x30
#define FI2C_MSG_RAW_INTR_STAT                   0x34
#define FI2C_MSG_RX_TL                           0x38
#define FI2C_MSG_TX_TL                           0x3C
#define FI2C_MSG_CLR_INTR                        0x40
#define FI2C_MSG_CLR_RX_UNDER                    0x44
#define FI2C_MSG_CLR_RX_OVER                     0x48
#define FI2C_MSG_CLR_TX_OVER                     0x4C
#define FI2C_MSG_CLR_RD_REQ                      0x50
#define FI2C_MSG_CLR_TX_ABRT                     0x54
#define FI2C_MSG_CLR_RX_DONE                     0x58
#define FI2C_MSG_CLR_ACTIVITY                    0x5C
#define FI2C_MSG_CLR_STOP_DET                    0x60
#define FI2C_MSG_CLR_START_DET                   0x64
#define FI2C_MSG_CLR_GEN_CALL                    0x68
#define FI2C_MSG_ENABLE                          0x6C
#define FI2C_MSG_STATUS                          0x70
#define FI2C_MSG_TXFLR                           0x74
#define FI2C_MSG_RXFLR                           0x78
#define FI2C_MSG_SDA_HOLD                        0x7C
#define FI2C_MSG_TX_ABRT_SOURCE                  0x80
#define FI2C_MSG_ENABLE_STATUS                   0x9C
#define FI2C_MSG_SMBCLK_LOW_MEXT                 0xA8
#define FI2C_MSG_SMBCLK_LOW_TIMEOUT              0xAC
#define FI2C_MSG_SMBDAT_STUCK_TIMEOUT            0xB4
#define FI2C_MSG_CLR_SMBCLK_EXT_LOW_TIMEOUT      0xBC
#define FI2C_MSG_CLR_SMBCLK_TMO_LOW_TIMEOUT      0xC0
#define FI2C_MSG_CLR_SMBDAT_LOW_TIMEOUT          0xC4
#define FI2C_MSG_CLR_SMBALERT_IN_N               0xD0

#define FI2C_MSG_INTR_RX_UNDER                   0x001
#define FI2C_MSG_INTR_RX_OVER                    0x002
#define FI2C_MSG_INTR_RX_FULL                    0x004
#define FI2C_MSG_INTR_TX_OVER                    0x008
#define FI2C_MSG_INTR_TX_EMPTY                   0x010
#define FI2C_MSG_INTR_RD_REQ                     0x020
#define FI2C_MSG_INTR_TX_ABRT                    0x040
#define FI2C_MSG_INTR_RX_DONE                    0x080
#define FI2C_MSG_INTR_ACTIVITY                   0x100
#define FI2C_MSG_INTR_STOP_DET                   0x200
#define FI2C_MSG_INTR_START_DET                  0x400
#define FI2C_MSG_INTR_GEN_CALL                   0x800
#define FI2C_MSG_INTR_SMBCLK_EXT_LOW_TIMEOUT     0x1000
#define FI2C_MSG_INTR_SMBCLK_TMO_LOW_TIMEOUT     0x2000
#define FI2C_MSG_INTR_SMBSDA_LOW_TIMEOUT         0x4000
#define FI2C_MSG_SMBALERT_IN_N                   0x20000

#define FI2C_MSG_INTR_DEFAULT_MASK \
    (FI2C_MSG_INTR_RX_FULL | FI2C_MSG_INTR_TX_ABRT | FI2C_MSG_INTR_STOP_DET)
#define FI2C_MSG_INTR_MASTER_MASK (FI2C_MSG_INTR_DEFAULT_MASK | FI2C_MSG_INTR_TX_EMPTY)
#define FI2C_MSG_INTR_SLAVE_MASK \
    (FI2C_MSG_INTR_DEFAULT_MASK | FI2C_MSG_INTR_RX_DONE | FI2C_MSG_INTR_RX_UNDER | FI2C_MSG_INTR_RD_REQ)
#define FI2C_MSG_INTR_SMBUS_MASK                                        \
    (FI2C_MSG_INTR_MASTER_MASK | FI2C_MSG_INTR_SMBCLK_EXT_LOW_TIMEOUT | \
     FI2C_MSG_INTR_SMBCLK_TMO_LOW_TIMEOUT | FI2C_MSG_INTR_SMBSDA_LOW_TIMEOUT)
#define FI2C_MSG_INTR_SMBUS_TIME_MASK \
    (FI2C_MSG_INTR_SMBCLK_EXT_LOW_TIMEOUT | FI2C_MSG_INTR_SMBCLK_TMO_LOW_TIMEOUT | FI2C_MSG_INTR_SMBSDA_LOW_TIMEOUT)

#define FI2C_MSG_STATUS_ACTIVITY              0x01
#define FI2C_MSG_STATUS_TFE                   BIT(2)
#define FI2C_MSG_STATUS_MASTER_ACTIVITY       BIT(5)
#define FI2C_MSG_STATUS_SLAVE_ACTIVITY        BIT(6)
#define FI2C_MSG_SDA_HOLD_RX_SHIFT            16
#define FI2C_MSG_SDA_HOLD_RX_MASK             GENMASK(23, FI2C_MSG_SDA_HOLD_RX_SHIFT)
#define FI2C_MSG_ERR_TX_ABRT                  3
#define FI2C_MSG_TAR_10BIT_ADDR_MASTER        BIT(12)
#define FI2C_MSG_COMP_PARAM_1_SPEED_MODE_HIGH (BIT(2) | BIT(3))
#define FI2C_MSG_COMP_PARAM_1_SPEED_MODE_MASK GENMASK(3, 2)
#define FI2C_MSG_STATUS_IDLE                  0x0
#define FI2C_MSG_STATUS_WRITE_IN_PROGRESS     0x1
#define FI2C_MSG_STATUS_READ_IN_PROGRESS      0x2

#define FI2C_MSG_ABRT_7BIT_ADDR_NOACK         0
#define FI2C_MSG_ABRT_10BIT_ADDR1_NOACK       1
#define FI2C_MSG_ABRT_10BIT_ADDR2_NOACK       2
#define FI2C_MSG_ABRT_TXDATA_NOACK            3
#define FI2C_MSG_ABRT_GCALL_NOACK             4
#define FI2C_MSG_ABRT_GCALL_READ              5
#define FI2C_MSG_ABRT_SBYTE_ACKDET            7
#define FI2C_MSG_ABRT_SBYTE_NORSTRT           9
#define FI2C_MSG_ABRT_10BIT_RD_NORSTRT        10
#define FI2C_MSG_ABRT_MASTER_DIS              11
#define FI2C_MSG_ABRT_LOST                    12
#define FI2C_MSG_ABRT_SLAVE_FLUSH_TXFIFO      13
#define FI2C_MSG_SLAVE_ARBLOST                14
#define FI2C_MSG_SLAVE_RD_INTX                15

#define FI2C_MSG_TX_ABRT_7BIT_ADDR_NOACK      (1U << FI2C_MSG_ABRT_7BIT_ADDR_NOACK)
#define FI2C_MSG_TX_ABRT_10BIT_ADDR1_NOACK    (1U << FI2C_MSG_ABRT_10BIT_ADDR1_NOACK)
#define FI2C_MSG_TX_ABRT_10BIT_ADDR2_NOACK    (1U << FI2C_MSG_ABRT_10BIT_ADDR2_NOACK)
#define FI2C_MSG_TX_ABRT_TXDATA_NOACK         (1U << FI2C_MSG_ABRT_TXDATA_NOACK)
#define FI2C_MSG_TX_ABRT_GCALL_NOACK          (1U << FI2C_MSG_ABRT_GCALL_NOACK)
#define FI2C_MSG_TX_ABRT_GCALL_READ           (1U << FI2C_MSG_ABRT_GCALL_READ)
#define FI2C_MSG_TX_ABRT_SBYTE_ACKDET         (1U << FI2C_MSG_ABRT_SBYTE_ACKDET)
#define FI2C_MSG_TX_ABRT_SBYTE_NORSTRT        (1U << FI2C_MSG_ABRT_SBYTE_NORSTRT)
#define FI2C_MSG_TX_ABRT_10BIT_RD_NORSTRT     (1U << FI2C_MSG_ABRT_10BIT_RD_NORSTRT)
#define FI2C_MSG_TX_ABRT_MASTER_DIS           (1U << FI2C_MSG_ABRT_MASTER_DIS)
#define FI2C_MSG_TX_ABRT_LOST                 (1U << FI2C_MSG_ABRT_LOST)
#define FI2C_MSG_TX_ABRT_SLAVE_RD_INTX        (1U << FI2C_MSG_SLAVE_RD_INTX)
#define FI2C_MSG_TX_ABRT_SLAVE_ARBLOST        (1U << FI2C_MSG_SLAVE_ARBLOST)
#define FI2C_MSG_TX_ABRT_SLAVE_FLUSH_TXFIFO   (1U << FI2C_MSG_ABRT_SLAVE_FLUSH_TXFIFO)

#define FI2C_MSG_TX_ABRT_NOACK                                               \
    (FI2C_MSG_TX_ABRT_7BIT_ADDR_NOACK | FI2C_MSG_TX_ABRT_10BIT_ADDR1_NOACK | \
     FI2C_MSG_TX_ABRT_10BIT_ADDR2_NOACK | FI2C_MSG_TX_ABRT_TXDATA_NOACK | FI2C_MSG_TX_ABRT_GCALL_NOACK)

#define FI2C_MSG_CONTROLLER_TYPE_I2C    0
#define FI2C_MSG_CONTROLLER_TYPE_SMBUS  1
#define FI2C_MSG_COMPLETE_OK            1
#define FI2C_MSG_COMPLETE_UNKNOWN       0

#define FI2C_MSG_CON_MASTER_MODE_MASK   0x01
#define FI2C_MSG_CON_SLAVE_MODE_MASK    1 << 6
#define FI2C_MSG_CON_RESTART_MASK       0x20
#define FI2C_MSG_CON_ADDR_MODE_MASK     1 << 3
#define FI2C_MSG_ADDR_7BIT_MODE         0
#define FI2C_MSG_TRANS_TIMEOUT          0xF0

#define FI2C_MSG_ACCESS_INTR_MASK       0x00000004
#define FI2C_MSG_DEFAULT_CLOCK_FREQENCY 100000000

#define FI2C_MSG_RESULT_IGNORE_LVL      0
#define FI2C_MSG_RESULT_WRITE_ISR_LVL   1
#define FI2C_MSG_RESULT_READ_ISR_LVL    2

#define FI2C_MSG_TRANS_FRAME_START      BIT(0)
#define FI2C_MSG_TRANS_FRAME_END        BIT(1)
#define FI2C_MSG_TRANS_FRAME_RESTART    BIT(2)
#define FI2C_MSG_REGFILE_TX_RING_OFFSET 8
#define FI2C_MSG_REGFILE_TX_RING_MASK   GENMASK(13, 8)


#ifdef __cplusplus
}
#endif

#endif /* FI2C_MSG_HW_H */