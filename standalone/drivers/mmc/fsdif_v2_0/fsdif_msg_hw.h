/*
* Copyright : (C) 2025 Phytium Information Technology, Inc.
* All Rights Reserved.
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
* FilePath: fsdif_msg_hw.h
* Date: 2025-04-10 16:20:52
* LastEditTime: 2025-04-10 16:20:52
* Description:  This file is for sdif user interface definition
*
* Modify History:
*  Ver   Who        Date         Changes
* ----- ------     --------    --------------------------------------
* 1.0   zhugengyu  2025/4/10    init commit
*/

#ifndef FSDIF_MSG_HW_H
#define FSDIF_MSG_HW_H

/***************************** Include Files *********************************/

#include "fdrivers_port.h"
#include "fmsg_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


/************************** Constant Definitions *****************************/
/** @name Register file Map
 *
 * Register file definition for a SD device.
 * @
 */
#define FSDIF_CARD_DETECT_OFFSET \
    FMSG_HARDWARE_PASS_THROUGH_0 /* the card detect register */
#define FSDIF_DATA_OFFSET \
    FMSG_HARDWARE_PASS_THROUGH_1                /* the data FIFO access register */
#define FSDIF_DEBUG_OFFSET    FMSG_CUSTOM_REG_3 /* the debug function register */

/** @name Message Map
 *
 * Message definition for a SD device.
 * @
 */

/** @name Event Map
 *
 * Event definition for a SD device.
 * @
 */
#define FSDIF_INT_CD_BIT      BIT(0)  /* RW Card detect (CD) */
#define FSDIF_INT_RE_BIT      BIT(1)  /* RW Response error (RE) */
#define FSDIF_INT_CMD_BIT     BIT(2)  /* RW Command done (CD) */
#define FSDIF_INT_DTO_BIT     BIT(3)  /* RW Data transfer over (DTO) */
#define FSDIF_INT_TXDR_BIT    BIT(4)  /* RW Transmit FIFO data request (TXDR) */
#define FSDIF_INT_RXDR_BIT    BIT(5)  /* RW Receive FIFO data request (RXDR) */
#define FSDIF_INT_RCRC_BIT    BIT(6)  /* RW Response CRC error (RCRC) */
#define FSDIF_INT_DCRC_BIT    BIT(7)  /* RW Data CRC error (DCRC) */
#define FSDIF_INT_RTO_BIT     BIT(8)  /* RW Response timeout (RTO) */
#define FSDIF_INT_DRTO_BIT    BIT(9)  /* RW Data read timeout (DRTO) */
#define FSDIF_INT_HTO_BIT     BIT(10) /* RW Data starvation-by-host timeout (HTO) */
#define FSDIF_INT_FRUN_BIT    BIT(11) /* RW FIFO underrun/overrun error (FRUN) */
#define FSDIF_INT_HLE_BIT     BIT(12) /* RW Hardware locked write error (HLE) */
#define FSDIF_INT_SBE_BCI_BIT BIT(13) /* RW Start-bit error (SBE) */
#define FSDIF_INT_ACD_BIT     BIT(14) /* RW Auto command done (ACD) */
#define FSDIF_INT_EBE_BIT     BIT(15) /* RW End-bit error (read)/Write no CRC (EBE) */
#define FSDIF_INT_SDIO_BIT    BIT(16) /* RW SDIO interrupt for card */

#define FSDIF_INT_ALL_BITS    GENMASK(16, 0)
#define FSDIF_INTS_CMD_MASK                                                          \
    (FSDIF_INT_RE_BIT | FSDIF_INT_CMD_BIT | FSDIF_INT_RCRC_BIT | FSDIF_INT_RTO_BIT | \
     FSDIF_INT_HTO_BIT | FSDIF_INT_HLE_BIT)
#define FSDIF_INTS_CMD_ERR_MASK                                                       \
    (FSDIF_INT_RTO_BIT | FSDIF_INT_RCRC_BIT | FSDIF_INT_RE_BIT | FSDIF_INT_DCRC_BIT | \
     FSDIF_INT_DRTO_BIT | FSDIF_INT_SBE_BCI_BIT)

#define FSDIF_INTS_DATA_MASK \
    (FSDIF_INT_DTO_BIT | FSDIF_INT_DCRC_BIT | FSDIF_INT_DRTO_BIT | FSDIF_INT_SBE_BCI_BIT)

/** @name FSDIF_DMAC_STATUS_OFFSET Register
 */
#define FSDIF_DMAC_STATUS_TI              BIT(0) /* RW 发送中断。表示链表的数据发送完成 */
#define FSDIF_DMAC_STATUS_RI              BIT(1) /* RW 接收中断。表示链表的数据接收完成 */
#define FSDIF_DMAC_STATUS_FBE             BIT(2) /* RW 致命总线错误中断 */
#define FSDIF_DMAC_STATUS_DU              BIT(4) /* RW 链表不可用中断 */
#define FSDIF_DMAC_STATUS_CES             BIT(5) /* RW 卡错误汇总 */
#define FSDIF_DMAC_STATUS_NIS             BIT(8) /* RW 正常中断汇总 */
#define FSDIF_DMAC_STATUS_AIS             BIT(9) /* RW 异常中断汇总 */
#define FSDIF_DMAC_STATUS_EB_GET(reg_val) GET_REG32_BITS((reg_val), 12, 10)
#define FSDIF_DMAC_STATUS_ALL_BITS        GENMASK(9, 0)

/** @name RAW Command
 */
#define FSDIF_CMD_START                   BIT(31) /* 启动命令 */
#define FSDIF_CMD_USE_HOLD_REG            BIT(29) /* 0: 旁路HOLD寄存器，1: 使能HOLD寄存器 */
#define FSDIF_CMD_VOLT_SWITCH             BIT(28) /* 0: 无电压转换，1: 有电压转换 */
#define FSDIF_CMD_CCS_EXP                 BIT(23)
#define FSDIF_CMD_CEATA_RD                BIT(22)
#define FSDIF_CMD_UPD_CLK                 BIT(21) /* 1：不发送指令，仅更新时钟寄存器的值到卡时钟域内 */
#define FSDIF_CMD_INIT \
    BIT(15) /* 0：在发送指令前不发送初始化序列（80 个周期） 1: 发送 */
#define FSDIF_CMD_STOP_ABORT BIT(14) /* 1：停止或中止命令，用于停止当前的数据传输 */
#define FSDIF_CMD_PRV_DATA_WAIT \
    BIT(13) /* 1：等待前面的数据传输完成后再发送指令 0: 立即发送命令 */
#define FSDIF_CMD_SEND_STOP             BIT(12) /* 1：在数据传送结束时发送停止命令 */
#define FSDIF_CMD_STRM_MODE             BIT(11)
#define FSDIF_CMD_DAT_WRITE             BIT(10) /* 0：读卡 1：写卡 */
#define FSDIF_CMD_DAT_EXP               BIT(9) /* 0：不等待数据传输, 1：等待数据传输 */
#define FSDIF_CMD_RESP_CRC              BIT(8) /* 1：检查响应 CRC */
#define FSDIF_CMD_RESP_LONG             BIT(7) /* 0：等待卡的短响应 1：等待卡的长响应 */
#define FSDIF_CMD_RESP_EXP              BIT(6) /* 1：等待卡的响应，0：命令不需要卡响应 */
#define FSDIF_CMD_INDX_SET(ind)         SET_REG32_BITS((ind), 5, 0) /* 命令索引号 */
#define FSDIF_CMD_INDX_GET(reg)         (GENMASK(5, 0) & reg)

#define FSDIF_DATA_BARRIER()            FDRIVER_DSB()

/**************************** Type Definitions *******************************/

/*****************************************************************************/
#define FSDIF_READ_REG32(addr, reg_off) FtIn32((addr) + (u32)(reg_off))
#define FSDIF_WRITE_REG32(addr, reg_off, reg_val) \
    FtOut32((addr) + (u32)(reg_off), (u32)(reg_val))

#ifdef __cplusplus
}
#endif

#endif /* FSDIF_HW_V2_H */