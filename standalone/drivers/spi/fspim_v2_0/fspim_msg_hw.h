/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fspim_msg_hw.h
 * Date: 2024-10-14 11:27:42
 * LastEditTime: 2024-10-24 17:46:03
 * Description:  This file is for providing spi msg hardware interface and predefined macros.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun   2024/10/24   first release
 */
#ifndef FSPIM_MSG_HW_H
#define FSPIM_MSG_HW_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "fspim_msg.h"
#include "fdrivers_port.h"
#include "fio.h"

#define FSPIM_BAUD_R_SCKDV(x) \
    (GENMASK(15, 0) & ((x) << 0)) /* SCKDV 为 2 ~ 65534 之间的任何偶数值 */
#define FSPIM_BAUD_R_SCKDV_MIN         2U
#define FSPIM_BAUD_R_SCKDV_MAX         65534U
#define FSPIM_BAUD_R_SCKDV_IS_VALID(x) (0 == (x) % 2)

#define FSPI_MSG_CTRL_TMOD_TR          0x0 /*xmit & recv*/
#define FSPI_MSG_CTRL_TMOD_TO          0x1 /*xmit only*/
#define FSPI_MSG_CTRL_TMOD_RO          0x2 /*recv only*/

#define FSPI_MSG_DEFAULT_CLK_DIV       0x10U /* 默认分频系数 */


#define FSPI_MSG_NUM_OF_CS             4U
#define FSPI_MSG_CHIP_CS_EN(cs) \
    (GENMASK(FSPI_MSG_NUM_OF_CS - 1, 0) << FSPI_MSG_NUM_OF_CS)
#define FSPI_MSG_CHIP_SEL(cs)          BIT(cs)

#define FSPI_MSG_REGFILE_S2M_INT_CLEAN (0x74)
#define FSPI_MSG_REGFILE_DEBUG         (0x58)

#define FSPI_MSG_REGFILE_DEBUG_VAL     BIT(0)
#define FSPI_MSG_REGFILE_ALIVE_VAL     BIT(1)

typedef enum
{
    FSPIM_1_BYTE = 1,

    FSPIM_MAX_BYTES_NUM
} FSpimTransByte;


typedef enum
{
    FSPI_MSG_MODE_0 = 0b00, /* cpol=0, cpha=0 */
    FSPI_MSG_MODE_1 = 0b01, /* cpol=0, cpha=1 */
    FSPI_MSG_MODE_2 = 0b10, /* cpol=1, cpha=0 */
    FSPI_MSG_MODE_3 = 0b11, /* cpol=1, cpha=1 */
} FSpimCphaCpolMode;

/*写 spi regfile*/
void FSpiMsgWriteRegfile(FSpiMsgCtrl *spi_msg_ctrl, u32 reg_off, u32 val);
/*读 spi regfile*/
u32 FSpiMsgReadRegfile(FSpiMsgCtrl *spi_msg_ctrl, u32 reg_off);
/* 设置spi分频系数*/
int FSpiMsgSetClkDiv(FSpiMsgCtrl *spi_msg_ctrl, u32 clk_div);
/* 设置SPI传输速率*/
int FSpiMsgSetSpeed(FSpiMsgCtrl *spi_msg_ctrl, u32 clk_freq);
/* 设置spi传输数据宽度*/
int FSpiMsgSetDataWidth(FSpiMsgCtrl *spi_msg_ctrl, u8 n_bytes);
/* 设置spi模式(时钟相位和时钟极性)*/
int FSpiMsgSetMode(FSpiMsgCtrl *spi_msg_ctrl, FSpimCphaCpolMode mode);
/*设置spi传输模式*/
int FSpiMsgSetTransMode(FSpiMsgCtrl *spi_msg_ctrl, u8 tmod);
/* 使能或关闭中断*/
int FSpiMsgMaskIntr(FSpiMsgCtrl *spi_msg_ctrl, u8 enable);
/* 使能或禁用SPI控制器*/
int FSpiMsgSetEnable(FSpiMsgCtrl *spi_msg_ctrl, u8 enable);

/* 设置spi默认参数*/
int FspiMsgSetDefault(FSpiMsgCtrl *spi_msg_ctrl);

/* 控制片选信号*/
int FSpiMsgSetChipSelection(FSpiMsgCtrl *spi_msg_ctrl, boolean enable);

void FSpiMsgSetDebug(FSpiMsgCtrl *spi_msg_ctrl, boolean enable);
void FSpiMsgSetAlive(FSpiMsgCtrl *spi_msg_ctrl, boolean enable);

/* 发送spi msg命令 */
int FSpiMsgSet(FSpiMsgCtrl *spi_msg_ctrl);

#ifdef __cplusplus
}
#endif

#endif // !