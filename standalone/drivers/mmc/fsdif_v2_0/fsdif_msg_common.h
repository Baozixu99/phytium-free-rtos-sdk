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
* FilePath: fsdif_msg_common.h
* Date: 2025-04-10 16:20:52
* LastEditTime: 2025-04-10 16:20:52
* Description:  This file is for sdif user interface definition
*
* Modify History:
*  Ver   Who        Date         Changes
* ----- ------     --------    --------------------------------------
* 1.0   zhugengyu  2025/4/10    init commit
*/

#ifndef FSDIF_MSG_COMMON_H
#define FSDIF_MSG_COMMON_H

/***************************** Include Files *********************************/

#include "fdrivers_port.h"
#include "fmsg_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
/** @name FMSG_CMD_SET Message
 */
/* 输入参数：clk_rate 时钟频率，caps 设备能力 */
#define FSDIF_MSG_CMD_SET_INIT              0x00 /* 初始化（软件初始化参数传递） */
#define FSDIF_MSG_CMD_SET_INIT_HW           0x01 /* 硬件初始化 */
#define FSDIF_MSG_CMD_SET_DEINIT_HW         0x02 /* 硬件去初始化 */
/* 输入参数： arg 命令参数 flags 命令标志 opcode 命令操作码 rawcmd 原生指令 */
#define FSDIF_MSG_CMD_SET_START_CMD         0x03 /* 指令传输 */
/* 输入参数： flag 数据flag adtc adtc类型 adma_addr_h adma_addr_l DMA地址
             blksz 块大小 blocks 块数目 arg 命令参数 rawcmd 原生指令 */
#define FSDIF_MSG_CMD_SET_START_DATA        0x04 /* 数据传输 */
/* 输入参数：timing 工作时序  bus_width 位宽 power_mode 上电模式 clock 工作频率 */
#define FSDIF_MSG_CMD_SET_SET_IOS           0x05 /* IOS设置 */
/* 输入参数：enable 使能状态 */
#define FSDIF_MSG_CMD_SET_EN_SDIO_IRQ       0x06 /* SDIO中断使能 */
/* 输入参数：signal_voltage 工作电压模式
   输出参数：state 电压切换结果 */
#define FSDIF_MSG_CMD_SET_SWITCH_VOLT       0x07 /* 工作电压设置 */

/** @name FMSG_MSG_CMD_DEFAULT Message
 */
#define FSDIF_CMD_DEFAULT_HW_RESET          0x00 /* 卡硬件复位 */
#define FSDIF_CMD_DEFAULT_ADMA_RESET        0x01 /* ADMA复位 */

/** @name FMSG_CMD_SET Message
 */
/* 输入参数：clk_rate 时钟频率，caps 设备能力 */
#define FSDIF_MSG_CMD_SET_INIT              0x00 /* 初始化（软件初始化参数传递） */
#define FSDIF_MSG_CMD_SET_INIT_HW           0x01 /* 硬件初始化 */
#define FSDIF_MSG_CMD_SET_DEINIT_HW         0x02 /* 硬件去初始化 */
/* 输入参数： arg 命令参数 flags 命令标志 opcode 命令操作码 rawcmd 原生指令 */
#define FSDIF_MSG_CMD_SET_START_CMD         0x03 /* 指令传输 */
/* 输入参数： flag 数据flag adtc adtc类型 adma_addr_h adma_addr_l DMA地址
             blksz 块大小 blocks 块数目 arg 命令参数 rawcmd 原生指令 */
#define FSDIF_MSG_CMD_SET_START_DATA        0x04 /* 数据传输 */
/* 输入参数：timing 工作时序  bus_width 位宽 power_mode 上电模式 clock 工作频率 */
#define FSDIF_MSG_CMD_SET_SET_IOS           0x05 /* IOS设置 */
/* 输入参数：enable 使能状态 */
#define FSDIF_MSG_CMD_SET_EN_SDIO_IRQ       0x06 /* SDIO中断使能 */
/* 输入参数：signal_voltage 工作电压模式
   输出参数：state 电压切换结果 */
#define FSDIF_MSG_CMD_SET_SWITCH_VOLT       0x07 /* 工作电压设置 */

/** @name FMSG_CMD_GET Message
 */
/* 输出参数： state 卡在位状态 */
#define FSDIF_MSG_CMD_GET_CD                0x00 /* 获取卡在位状态 */
/* 输出参数： state 卡繁忙状态 */
#define FSDIF_MSG_CMD_GET_CARD_BUSY         0x01 /* 获取卡繁忙状态 */
#define FSDIF_MSG_CMD_GET_STATUS            0x02
#define FSDIF_MSG_CMD_GET_READ_ONLY         0x03

/** @name FMSG_CMD_DATA Message
 */

/** @name FMSG_CMD_REPORT Message
 */
#define FSDIF_MSG_CMD_REPORT_CD_IRQ         0x00 /* card_detect 中断触发热插拔功能 */
/* 输出参数： cmd_err CMD异常状态 dma_err DMA异常状态 */
#define FSDIF_MSG_CMD_REPORT_ERR_IRQ        0x01 /* DMA/CMD 异常中断 */
/* 输出参数： events 触发的中断事件 resp[4] 响应的返回结果 */
#define FSDIF_MSG_CMD_REPORT_CMD_NEXT       0x02 /* CMD 完成后 CMD NEXT 处理 */
/* 输出参数： events 触发的中断事件 */
#define FSDIF_MSG_CMD_REPORT_DATA_NEXT      0x03 /* DATA 完成后 DATA NEXT 处理 */

/** @name FMSG_CMD_PROTOCOL Message
 */
#define FSDIF_MSG_STATUS0_NOT_READY         0x00
#define FSDIF_MSG_STATUS0_SUCCESS           0x01
#define FSDIF_MSG_STATUS0_GOING             0x02
#define FSDIF_MSG_STATUS0_GENERIC_ERROR     0x10
#define FSDIF_MSG_STATUS0_TYPE_NOT_SUPPORT  0x11
#define FSDIF_MSG_STATUS0_CMD_NOT_SUPPORT   0x12
#define FSDIF_MSG_STATUS0_INVALID_PARAMETER 0x13

#define FSDIF_MSG_STATUS1_SUCCESS           0

/**************************** Type Definitions *******************************/
typedef struct
{
    u32 caps;
#define FSDIF_MMC_CAP_4_BIT_DATA      (1 << 0)  /* Can the host do 4 bit transfers */
#define FSDIF_MMC_CAP_MMC_HIGHSPEED   (1 << 1)  /* Can do MMC high-speed timing */
#define FSDIF_MMC_CAP_SD_HIGHSPEED    (1 << 2)  /* Can do SD high-speed timing */
#define FSDIF_MMC_CAP_SDIO_IRQ        (1 << 3)  /* Can signal pending SDIO IRQs */
#define FSDIF_MMC_CAP_SPI             (1 << 4)  /* Talks only SPI protocols */
#define FSDIF_MMC_CAP_NEEDS_POLL      (1 << 5)  /* Needs polling for card-detection */
#define FSDIF_MMC_CAP_8_BIT_DATA      (1 << 6)  /* Can the host do 8 bit transfers */
#define FSDIF_MMC_CAP_AGGRESSIVE_PM   (1 << 7)  /* Suspend (e)MMC/SD at idle  */
#define FSDIF_MMC_CAP_NONREMOVABLE    (1 << 8)  /* Nonremovable e.g. eMMC */
#define FSDIF_MMC_CAP_WAIT_WHILE_BUSY (1 << 9)  /* Waits while card is busy */
#define FSDIF_MMC_CAP_3_3V_DDR        (1 << 11) /* Host supports eMMC DDR 3.3V */
#define FSDIF_MMC_CAP_1_8V_DDR        (1 << 12) /* Host supports eMMC DDR 1.8V */
#define FSDIF_MMC_CAP_1_2V_DDR        (1 << 13) /* Host supports eMMC DDR 1.2V */
#define FSDIF_MMC_CAP_DDR \
    (FSDIF_MMC_CAP_3_3V_DDR | FSDIF_MMC_CAP_1_8V_DDR | FSDIF_MMC_CAP_1_2V_DDR)
#define FSDIF_MMC_CAP_POWER_OFF_CARD (1 << 14) /* Can power off after boot */
#define FSDIF_MMC_CAP_BUS_WIDTH_TEST (1 << 15) /* CMD14/CMD19 bus width ok */
#define FSDIF_MMC_CAP_UHS_SDR12      (1 << 16) /* Host supports UHS SDR12 mode */
#define FSDIF_MMC_CAP_UHS_SDR25      (1 << 17) /* Host supports UHS SDR25 mode */
#define FSDIF_MMC_CAP_UHS_SDR50      (1 << 18) /* Host supports UHS SDR50 mode */
#define FSDIF_MMC_CAP_UHS_SDR104     (1 << 19) /* Host supports UHS SDR104 mode */
#define FSDIF_MMC_CAP_UHS_DDR50      (1 << 20) /* Host supports UHS DDR50 mode */
#define FSDIF_MMC_CAP_UHS                                                          \
    (FSDIF_MMC_CAP_UHS_SDR12 | FSDIF_MMC_CAP_UHS_SDR25 | FSDIF_MMC_CAP_UHS_SDR50 | \
     FSDIF_MMC_CAP_UHS_SDR104 | FSDIF_MMC_CAP_UHS_DDR50)
#define FSDIF_MMC_CAP_SYNC_RUNTIME_PM (1 << 21) /* Synced runtime PM suspends. */
#define FSDIF_MMC_CAP_NEED_RSP_BUSY   (1 << 22) /* Commands with R1B can't use R1. */
#define FSDIF_MMC_CAP_DRIVER_TYPE_A   (1 << 23) /* Host supports Driver Type A */
#define FSDIF_MMC_CAP_DRIVER_TYPE_C   (1 << 24) /* Host supports Driver Type C */
#define FSDIF_MMC_CAP_DRIVER_TYPE_D   (1 << 25) /* Host supports Driver Type D */
#define FSDIF_MMC_CAP_DONE_COMPLETE \
    (1 << 27) /* RW reqs can be completed within mmc_request_done() */
#define FSDIF_MMC_CAP_CD_WAKE        (1 << 28) /* Enable card detect wake */
#define FSDIF_MMC_CAP_CMD_DURING_TFR (1 << 29) /* Commands during data transfer */
#define FSDIF_MMC_CAP_CMD23          (1 << 30) /* CMD23 supported. */
#define FSDIF_MMC_CAP_HW_RESET       (1 << 31) /* Reset the eMMC card via RST_n */
    u32 clk_rate;
} FSdifMsgDataInit;

typedef struct
{
    u32 ios_clock;
    u8 ios_timing;
#define FSDIF_MMC_TIMING_LEGACY      0
#define FSDIF_MMC_TIMING_MMC_HS      1
#define FSDIF_MMC_TIMING_SD_HS       2
#define FSDIF_MMC_TIMING_UHS_SDR12   3
#define FSDIF_MMC_TIMING_UHS_SDR25   4
#define FSDIF_MMC_TIMING_UHS_SDR50   5
#define FSDIF_MMC_TIMING_UHS_SDR104  6
#define FSDIF_MMC_TIMING_UHS_DDR50   7
#define FSDIF_MMC_TIMING_MMC_DDR52   8
#define FSDIF_MMC_TIMING_MMC_HS200   9
#define FSDIF_MMC_TIMING_MMC_HS400   10
#define FSDIF_MMC_TIMING_SD_EXP      11
#define FSDIF_MMC_TIMING_SD_EXP_1_2V 12
    u8 ios_bus_width;
#define FSDIF_MMC_BUS_WIDTH_1 0
#define FSDIF_MMC_BUS_WIDTH_4 2
#define FSDIF_MMC_BUS_WIDTH_8 3
    u8 ios_power_mode;
#define FSDIF_MMC_POWER_OFF       0
#define FSDIF_MMC_POWER_UP        1
#define FSDIF_MMC_POWER_ON        2
#define FSDIF_MMC_POWER_UNDEFINED 3
} FSdifMsgDataSetIos;

typedef struct
{
    unsigned char signal_voltage; /* signalling voltage (1.8V or 3.3V) */
#define FSDIF_MMC_SIGNAL_VOLTAGE_330 0
#define FSDIF_MMC_SIGNAL_VOLTAGE_180 1
#define FSDIF_MMC_SIGNAL_VOLTAGE_120 2
} FSdifMsgDataSwitchVolt;

typedef struct
{
    u32 cmd_arg;
    u32 flags;
#define FSDIF_MMC_RSP_PRESENT  (1 << 0)
#define FSDIF_MMC_RSP_136      (1 << 1) /* 136 bit response */
#define FSDIF_MMC_RSP_CRC      (1 << 2) /* expect valid crc */
#define FSDIF_MMC_RSP_BUSY     (1 << 3) /* card may send busy */
#define FSDIF_MMC_RSP_OPCODE   (1 << 4) /* response contains opcode */

#define FSDIF_MMC_CMD_MASK     (3 << 5) /* non-SPI command type */
#define FSDIF_MMC_CMD_AC       (0 << 5)
#define FSDIF_MMC_CMD_ADTC     (1 << 5)
#define FSDIF_MMC_CMD_BC       (2 << 5)
#define FSDIF_MMC_CMD_BCR      (3 << 5)

#define FSDIF_MMC_RSP_SPI_S1   (1 << 7)  /* one status byte */
#define FSDIF_MMC_RSP_SPI_S2   (1 << 8)  /* second byte */
#define FSDIF_MMC_RSP_SPI_B4   (1 << 9)  /* four data bytes */
#define FSDIF_MMC_RSP_SPI_BUSY (1 << 10) /* card may send busy */

/*
 * These are the native response types, and correspond to valid bit
 * patterns of the above flags.  One additional valid pattern
 * is all zeros, which means we don't expect a response.
 */
#define FSDIF_MMC_RSP_NONE     (0)
#define FSDIF_MMC_RSP_R1 \
    (FSDIF_MMC_RSP_PRESENT | FSDIF_MMC_RSP_CRC | FSDIF_MMC_RSP_OPCODE)
#define FSDIF_MMC_RSP_R1B \
    (FSDIF_MMC_RSP_PRESENT | FSDIF_MMC_RSP_CRC | FSDIF_MMC_RSP_OPCODE | FSDIF_MMC_RSP_BUSY)
#define FSDIF_MMC_RSP_R2 (FSDIF_MMC_RSP_PRESENT | FSDIF_MMC_RSP_136 | FSDIF_MMC_RSP_CRC)
#define FSDIF_MMC_RSP_R3 (FSDIF_MMC_RSP_PRESENT)
#define FSDIF_MMC_RSP_R4 (FSDIF_MMC_RSP_PRESENT)
#define FSDIF_MMC_RSP_R5 \
    (FSDIF_MMC_RSP_PRESENT | FSDIF_MMC_RSP_CRC | FSDIF_MMC_RSP_OPCODE)
#define FSDIF_MMC_RSP_R6 \
    (FSDIF_MMC_RSP_PRESENT | FSDIF_MMC_RSP_CRC | FSDIF_MMC_RSP_OPCODE)
#define FSDIF_MMC_RSP_R7 \
    (FSDIF_MMC_RSP_PRESENT | FSDIF_MMC_RSP_CRC | FSDIF_MMC_RSP_OPCODE)

/* Can be used by core to poll after switch to MMC HS mode */
#define FSDIF_MMC_RSP_R1_NO_CRC (FSDIF_MMC_RSP_PRESENT | FSDIF_MMC_RSP_OPCODE)

#define fsdif_mmc_resp_type(cmd)                                                     \
    ((cmd)->flags & (FSDIF_MMC_RSP_PRESENT | FSDIF_MMC_RSP_136 | FSDIF_MMC_RSP_CRC | \
                     FSDIF_MMC_RSP_BUSY | FSDIF_MMC_RSP_OPCODE))

/*
 * These are the SPI response types for MMC, SD, and SDIO cards.
 * Commands return R1, with maybe more info.  Zero is an error type;
 * callers must always provide the appropriate FSDIF_MMC_RSP_SPI_Rx flags.
 */
#define FSDIF_MMC_RSP_SPI_R1  (FSDIF_MMC_RSP_SPI_S1)
#define FSDIF_MMC_RSP_SPI_R1B (FSDIF_MMC_RSP_SPI_S1 | FSDIF_MMC_RSP_SPI_BUSY)
#define FSDIF_MMC_RSP_SPI_R2  (FSDIF_MMC_RSP_SPI_S1 | FSDIF_MMC_RSP_SPI_S2)
#define FSDIF_MMC_RSP_SPI_R3  (FSDIF_MMC_RSP_SPI_S1 | FSDIF_MMC_RSP_SPI_B4)
#define FSDIF_MMC_RSP_SPI_R4  (FSDIF_MMC_RSP_SPI_S1 | FSDIF_MMC_RSP_SPI_B4)
#define FSDIF_MMC_RSP_SPI_R5  (FSDIF_MMC_RSP_SPI_S1 | FSDIF_MMC_RSP_SPI_S2)
#define FSDIF_MMC_RSP_SPI_R7  (FSDIF_MMC_RSP_SPI_S1 | FSDIF_MMC_RSP_SPI_B4)

#define fsdif_mmc_spi_resp_type(cmd)                                 \
    ((cmd)->flags & (FSDIF_MMC_RSP_SPI_S1 | FSDIF_MMC_RSP_SPI_BUSY | \
                     FSDIF_MMC_RSP_SPI_S2 | FSDIF_MMC_RSP_SPI_B4))

/*
 * These are the command types.
 */
#define fsidf_mmc_cmd_type(cmd) ((cmd)->flags & FSDIF_MMC_CMD_MASK)
    u32 opcode;
#define SD_SWITCH_VOLTAGE 11U
#define SD_EXT_APP_CMD    55U
    u32 raw_cmd;
} FSdifMsgDataStartCmd;

typedef struct
{
    u32 data_flags;
#define FSDIF_MMC_DATA_WRITE      BIT(8)
#define FSDIF_MMC_DATA_READ       BIT(9)
/* Extra flags used by CQE */
#define FSDIF_MMC_DATA_QBR        BIT(10) /* CQE queue barrier*/
#define FSDIF_MMC_DATA_PRIO       BIT(11) /* CQE high priority */
#define FSDIF_MMC_DATA_REL_WR     BIT(12) /* Reliable write */
#define FSDIF_MMC_DATA_DAT_TAG    BIT(13) /* Tag request */
#define FSDIF_MMC_DATA_FORCED_PRG BIT(14) /* Forced programming */
    u32 adtc_type;
#define FSDIF_COMMOM_ADTC   0
#define FSDIF_BLOCK_RW_ADTC 1
    u64 adma_addr;
    u32 mrq_data_blksz;
    u32 mrq_data_blocks;
    u32 cmd_arg;
    u32 raw_cmd;
} FSdifMsgDataStartData;

typedef struct
{
    u32 events;
    u32 response0;
    u32 response1;
    u32 response2;
    u32 response3;
} FSdifMsgDataCmdNext;

typedef struct
{
    u32 raw_ints;
    u32 ints_mask;
    u32 dmac_status;
    u32 dmac_mask;
    u32 opcode;
} FSdifMsgDataErrIrq;

typedef struct
{
    u8 reserved;
    u8 seq;
    u8 cmd_type;
    u8 cmd_subid;
    u16 len;
    u8 status1;
    u8 status0;
    u8 para[56];
} __attribute__((packed)) FSdifMsgInfo;

typedef struct
{
    FSdifMsgInfo *tx_shmem[FMSG_MAX_RING_ITEMS];
    FSdifMsgInfo *rv_shmem[FMSG_MAX_RING_ITEMS];
    uintptr regfile;
    uintptr shmem;
} FSdifMsgDev;
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* FSDIF_MSG_V2_H */