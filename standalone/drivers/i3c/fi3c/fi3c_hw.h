/*
 * Copyright (C) 2021, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fi3c_hw.h
 * Date: 2021-11-01 14:53:42
 * LastEditTime: 2022-02-18 08:36:32
 * Description:  This file is for I3C register definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhangyan 2023/9/11  first commit
 */
#ifndef FI3C_HW_H
#define FI3C_HW_H
/***************************** Include Files *********************************/
#include "fparameters.h"
#include "fio.h"
#include "ferror_code.h"
#include "fdrivers_port.h"

#ifdef __cplusplus
extern "C"
{
#endif
/************************** Constant Definitions *****************************/
#define FI3C_HOT_JOIN_ADDR                     0x2
#define FI3C_BROADCAST_ADDR                    0x7e
#define FI3C_DEV_ID_MASTER                     0x5034
#define FI3C_DEV_ID_OFFSET                     0x00
#define FI3C_IBIR_OFFSET                       0x3C
#define FI3C_TX_FIFO_OFFSET                    0x68
#define FI3C_IMD_CMD0_OFFSET                   0x70
#define FI3C_IMD_CMD1_OFFSET                   0x74
#define FI3C_IMD_DATA_OFFSET                   0x78
#define FI3C_RX_FIFO_OFFSET                    0x80
#define FI3C_IBI_DATA_FIFO_OFFSET              0x84
#define FI3C_CMD_IBI_THR_CTRL_OFFSET           0x90
#define FI3C_TX_RX_THR_CTRL_OFFSET             0x94

/** @name FI3C_CONF_STATUS0_OFFSET Register
 */
#define FI3C_CONF_STATUS0_OFFSET               0x04
#define FI3C_CONF_STATUS0_CMDR_DEPTH(reg_val)  (4 << (((reg_val)&GENMASK(31, 29)) >> 29))
#define FI3C_CONF_STATUS0_ECC_CHK_MASK         BIT(28)
#define FI3C_CONF_STATUS0_INTEG_CHK_MASK       BIT(27)
#define FI3C_CONF_STATUS0_CSR_DAP_CHK_MASK     BIT(26)
#define FI3C_CONF_STATUS0_TRANS_TOUT_CHK_MASK  BIT(25)
#define FI3C_CONF_STATUS0_PROT_FAULTS_CHK_MASK BIT(24)
#define FI3C_CONF_STATUS0_GPO_NUM(reg_val)     (((reg_val)&GENMASK(23, 16)) >> 16)
#define FI3C_CONF_STATUS0_GPI_NUM(reg_val)     (((reg_val)&GENMASK(15, 8)) >> 8)
#define FI3C_CONF_STATUS0_IBIR_DEPTH(reg_val)  (4 << (((reg_val)&GENMASK(7, 6)) >> 7))
#define FI3C_CONF_STATUS0_SUPPORTS_DDR_MASK    BIT(5)
#define FI3C_CONF_STATUS0_SEC_MASTER_MASK      BIT(4)
#define FI3C_CONF_STATUS0_DEVS_NUM(reg_val)    ((reg_val)&GENMASK(3, 0))

/** @name FI3C_CONF_STATUS1_OFFSET Register
 */
#define FI3C_CONF_STATUS1_OFFSET               0x08
#define FI3C_CONF_STATUS1_IBI_HW_RES(reg_val)  ((((reg_val)&GENMASK(31, 28)) >> 28) + 1)
#define FI3C_CONF_STATUS1_CMD_DEPTH(reg_val)   (4 << (((reg_val)&GENMASK(27, 26)) >> 26))
#define FI3C_CONF_STATUS1_SLVDDR_RX_DEPTH(reg_val) \
    (8 << (((reg_val)&GENMASK(25, 21)) >> 21))
#define FI3C_CONF_STATUS1_SLVDDR_TX_DEPTH(reg_val) \
    (8 << (((reg_val)&GENMASK(20, 16)) >> 16))
#define FI3C_CONF_STATUS1_IBI_DEPTH(reg_val) (2 << (((x)&GENMASK(12, 10)) >> 10))
#define FI3C_CONF_STATUS1_RX_DEPTH(reg_val)  (8 << (((reg_val)&GENMASK(9, 5)) >> 5))
#define FI3C_CONF_STATUS1_TX_DEPTH(reg_val)  (8 << ((reg_val)&GENMASK(4, 0)))

/** @name FI3C_CTRL_OFFSET Register
 */
#define FI3C_CTRL_OFFSET                     0x10
#define FI3C_CTRL_DEV_EN                     BIT(31)
#define FI3C_CTRL_HALT_EN                    BIT(30)
#define FI3C_CTRL_MCS                        BIT(29)
#define FI3C_CTRL_MCS_EN                     BIT(28)
#define FI3C_CTRL_THD_DELAY(date)            (((date) << 24) & GENMASK(25, 24))
#define FI3C_CTRL_HJ_DISEC                   BIT(8)
#define FI3C_CTRL_MST_ACK                    BIT(7)
#define FI3C_CTRL_JI_ACK                     BIT(6)

/** @name FI3C_PRESCL_CTRL0_OFFSET Register
 */
#define FI3C_PRESCL_CTRL0_OFFSET             0x14
#define FI3C_PRESCL_CTRL0_I2C(date)          ((date) << 16)
#define FI3C_PRESCL_CTRL0_I3C(date)          (date)
#define FI3C_PRESCL_CTRL0_I2C_MASK           GENMASK(31, 16)
#define FI3C_PRESCL_CTRL0_I3C_MASK           GENMASK(9, 0)

/** @name FI3C_PRESCL_CTRL1_OFFSET Register
 */
#define FI3C_PRESCL_CTRL1_OFFSET             0x18
#define FI3C_PRESCL_CTRL1_PP_LOW_MASK        GENMASK(15, 8)
#define FI3C_PRESCL_CTRL1_PP_LOW(x)          ((x) << 8)
#define FI3C_PRESCL_CTRL1_OD_LOW_MASK        GENMASK(7, 0)
#define FI3C_PRESCL_CTRL1_OD_LOW(x)          (x)

/** @name FI3C interrupt register
 */
#define FI3C_MST_IER_OFFSET                  0x20
#define FI3C_MST_IDR_OFFSET                  0x24
#define FI3C_MST_IMR_OFFSET                  0x28
#define FI3C_MST_ICR_OFFSET                  0x2C
#define FI3C_MST_ISR_OFFSET                  0x30
#define FI3C_MST_INT_HALTED_MASK             BIT(18)
#define FI3C_MST_INT_MR_DONE_MASK            BIT(17)
#define FI3C_MST_INT_IMM_COMP_MASK           BIT(16)
#define FI3C_MST_INT_TX_THR_MASK             BIT(15)
#define FI3C_MST_INT_TX_OVF_MASK             BIT(14)
#define FI3C_MST_INT_IBID_THR_MASK           BIT(12)
#define FI3C_MST_INT_IBID_UNF_MASK           BIT(11)
#define FI3C_MST_INT_IBIR_THR_MASK           BIT(10)
#define FI3C_MST_INT_IBIR_UNF_MASK           BIT(9)
#define FI3C_MST_INT_IBIR_OVF_MASK           BIT(8)
#define FI3C_MST_INT_RX_THR_MASK             BIT(7)
#define FI3C_MST_INT_RX_UNF_MASK             BIT(6)
#define FI3C_MST_INT_CMDD_EMP_MASK           BIT(5)
#define FI3C_MST_INT_CMDD_THR_MASK           BIT(4)
#define FI3C_MST_INT_CMDD_OVF_MASK           BIT(3)
#define FI3C_MST_INT_CMDR_THR_MASK           BIT(2)
#define FI3C_MST_INT_CMDR_UNF_MASK           BIT(1)
#define FI3C_MST_INT_CMDR_OVF_MASK           BIT(0)
#define FI3C_MST_INT_ALL_BIT_MASK            GENMASK(18, 0)

/** @name FI3C_MST_STATUS0_OFFSET Register
 */
#define FI3C_MST_STATUS0_OFFSET              0x34
#define FI3C_MST_STATUS0_IDLE                BIT(18)
#define FI3C_MST_STATUS0_HALTED              BIT(17)
#define FI3C_MST_STATUS0_MASTER_MODE         BIT(16)
#define FI3C_MST_STATUS0_TX_FULL             BIT(13)
#define FI3C_MST_STATUS0_IBID_FULL           BIT(12)
#define FI3C_MST_STATUS0_IBIR_FULL           BIT(11)
#define FI3C_MST_STATUS0_RX_FULL             BIT(10)
#define FI3C_MST_STATUS0_CMDD_FULL           BIT(9)
#define FI3C_MST_STATUS0_CMDR_FULL           BIT(8)
#define FI3C_MST_STATUS0_TX_EMP              BIT(5)
#define FI3C_MST_STATUS0_IBID_EMP            BIT(4)
#define FI3C_MST_STATUS0_IBIR_EMP            BIT(3)
#define FI3C_MST_STATUS0_RX_EMP              BIT(2)
#define FI3C_MST_STATUS0_CMDD_EMP            BIT(1)
#define FI3C_MST_STATUS0_CMDR_EMP            BIT(0)

/** @name FI3C_CMDR_OFFSET Register
 */
#define FI3C_CMDR_OFFSET                     0x38
#define FI3C_CMDR_CMDID_MASK                 GENMASK(7, 0)
#define FI3C_CMDR_XFER_BYTES_MASK            GENMASK(19, 8)
#define FI3C_CMDR_ERROR_MASK                 GENMASK(27, 24)

/** @name FI3C_CMD0_FIFO_OFFSET Register
 */
#define FI3C_CMD0_FIFO_OFFSET                0x60
#define FI3C_CMD0_FIFO_IS_DDR(data)          ((data) << 31)
#define FI3C_CMD0_FIFO_IS_CCC(data)          ((data) << 30)
#define FI3C_CMD0_FIFO_BCH(data)             ((data) << 29)
#define FI3C_CMD0_FIFO_PRIV_XMIT_MODE(data)  ((data) << 27)
#define FI3C_CMD0_FIFO_SBCA(data)            ((data) << 26)
#define FI3C_CMD0_FIFO_RSBC(data)            ((data) << 25)
#define FI3C_CMD0_FIFO_IS_10B(data)          ((data) << 24)
#define FI3C_CMD0_FIFO_PL_LEN(data)          ((data) << 12)
#define FI3C_CMD0_FIFO_DEV_ADDR(data)        ((data) << 1)
#define FI3C_CMD0_FIFO_RNW(data)             ((data) << 0)

#define FI3C_CMD0_FIFO_RNW_ENABLE            1
#define FI3C_CMD0_FIFO_PL_LEN_MAX            4095

/** @name FI3C_CMD1_FIFO_OFFSET Register
 */
#define FI3C_CMD1_FIFO_OFFSET                0x64
#define FI3C_CMD1_FIFO_CMDID(date)           ((date) << 24)
#define FI3C_CMD1_FIFO_CCC_CSRADDR(date)     ((date))

/** @name FI3C_FLUSH_CTRL_OFFSET Register
 */
#define FI3C_FLUSH_CTRL_OFFSET               0x9C
#define FI3C_FLUSH_IBI_RESP_MASK             BIT(24)
#define FI3C_FLUSH_CMD_RESP_MASK             BIT(23)
#define FI3C_FLUSH_SLV_DDR_RX_FIFO_MASK      BIT(22)
#define FI3C_FLUSH_SLV_DDR_TX_FIFO_MASK      BIT(21)
#define FI3C_FLUSH_IMM_FIFO_MASK             BIT(20)
#define FI3C_FLUSH_IBI_FIFO_MASK             BIT(19)
#define FI3C_FLUSH_RX_FIFO_MASK              BIT(18)
#define FI3C_FLUSH_TX_FIFO_MASK              BIT(17)
#define FI3C_FLUSH_CMD_FIFO_MASK             BIT(16)

/** @name  FI3C_DEVS_CTRL_OFFSET Register
 */
#define FI3C_DEVS_CTRL_OFFSET                0xB8
#define FI3C_DEV_CLR_ALL                     GENMASK(27, 17)
#define FI3C_DEV_ACTIVE_MASK                 GENMASK(11, 0)
#define FI3C_DEV_ACTIVE(dev)                 BIT(dev)

/** @name  Device Retaining Register
 */
#define FI3C_DEV_ID_RR0(dev)                 (0xC0 + ((dev)*0x10))
#define FI3C_DEV_ID_RR1(dev)                 (0xC4 + ((dev)*0x10))
#define FI3C_DEV_ID_RR2(dev)                 (0xC8 + ((dev)*0x10))
/*DEV_ID_RR0*/
#define FI3C_DEV_DEV_ADDR_MASK               GENMASK(7, 1)
#define FI3C_DEV_IS_I3C                      BIT(9)
#define FI3C_DEV_LVR_EXT_ADDR                BIT(11)
#define FI3C_DEV_IVR_SA_MSB_MASK             GENMASK(15, 13)
/*DEV_ID_RR1*/
#define FI3C_DEV_PID_MSB_MASK                GENMASK(31, 0)
/*DEV_ID_RR2*/
#define FI3C_DEV_PID_LSB_MASK                GENMASK(31, 16)
#define FI3C_DEV_BCR_MASK                    GENMASK(15, 8)
#define FI3C_DCR_LVR_MASK                    GENMASK(7, 0)

/** @name FI3C_SIR_MAPX_OFFSET Register
 */
#define FI3C_SIR_MAPX_OFFSET(x)              (0x180 + ((x)*4))
#define FI3C_SIR_MAP_DEV_REG(dev)            FI3C_SIR_MAPX_OFFSET((dev) / 2)
#define FI3C_SIR_MAP_DEV_SHIFT(d, fs)        ((fs) + (((d) % 2) ? 16 : 0))
#define FI3C_SIR_MAP_DEV_CONF_MASK(d)        (GENMASK(15, 0) << (((d) % 2) ? 16 : 0))
#define FI3C_SIR_MAP_DEV_CONF(d, c)          ((c) << (((d) % 2) ? 16 : 0))
#define FI3C_SIR_MAP_DEV_ROLE(role)          ((role) << 14)
#define FI3C_SIR_MAP_DEV_SLOW                BIT(13)
#define FI3C_SIR_MAP_DEV_PL(l)               ((l) << 8)
#define FI3C_SIR_MAP_PL_MAX                  GENMASK(4, 0)
#define FI3C_SIR_MAP_DEV_DA(a)               ((a) << 1)
#define FI3C_SIR_MAP_DEV_ACK                 BIT(0)
#define FI3C_BCR_MAX_DATA_SPEED_LIM          BIT(0)


/** @name other define 
 */
#define FI3C_READ_REG32(addr, reg_offset)    FtIn32((addr) + (u32)(reg_offset))
#define FI3C_WRITE_REG32(addr, reg_offset, reg_value) \
    FtOut32((addr) + (u32)(reg_offset), (u32)(reg_value))

#define FI3C_WRITE_TX_FIFO(addr, data) \
    FI3C_WRITE_REG32((addr), FI3C_TX_FIFO_OFFSET, (data));
#define FI3C_READ_RX_FIFO(addr) FI3C_READ_REG32((addr), FI3C_RX_FIFO_OFFSET)
#define FI3C_SETBIT(base_addr, reg_offset, data) \
    FtSetBit32((base_addr) + (u32)(reg_offset), (u32)(data))
#define FI3C_CLEARBIT(base_addr, reg_offset, data) \
    FtClearBit32((base_addr) + (u32)(reg_offset), (u32)(data))

FError FI3cSetEnable(uintptr addr, boolean enable);

#ifdef __cplusplus
}
#endif

#endif