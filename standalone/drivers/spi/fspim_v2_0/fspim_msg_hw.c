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
 * FilePath: fspim_msg_hw.c
 * Date: 2024-10-14 11:27:42
 * LastEditTime: 2024-10-24 17:46:03
 * Description:  This file is for providing spi msg config set functions definition.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun   2024/10/24   first release
 */
#include <string.h>
#include "fspim_msg.h"
#include "fspim_msg_hw.h"
#include "fmsg_common.h"
#include "ftypes.h"
#include "fassert.h"
#include "fdrivers_port.h"

#define FSPIM_MSG_DEBUG_TAG "FSPI-MSG-HW"
#define FSPIM_MSG_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FSPIM_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_MSG_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FSPIM_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_MSG_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FSPIM_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_MSG_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FSPIM_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
/***************** Macros (Inline Functions) Definitions *********************/

/**
 * @name: FSpiMsgWriteRegfile
 * @msg: 写spi regfile寄存器
 * @return {none}
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 * @param {u32} reg_off, regfile寄存器偏移地址
 * @param {u32} val, 寄存器值
 */

void FSpiMsgWriteRegfile(FSpiMsgCtrl *spi_msg_ctrl, u32 reg_off, u32 val)
{
    uintptr regfile = spi_msg_ctrl->spi_msg_config.spi_msg.regfile;
    FtOut32(regfile + reg_off, val);
}

/**
 * @name: FSpiMsgReadRegfile
 * @msg: 读spi regfile寄存器
 * @return {u32}, 返回读到的寄存器值
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 * @param {u32} reg_off, regfile寄存器偏移地址
 */

u32 FSpiMsgReadRegfile(FSpiMsgCtrl *spi_msg_ctrl, u32 reg_off)
{
    uintptr regfile = spi_msg_ctrl->spi_msg_config.spi_msg.regfile;
    return FtIn32(regfile + reg_off);
}

/**
 * @name: FSpiMsgSetDebug
 * @msg: 启用或禁用spi msg debug模式
 * @return {none}
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针， enable为true时启用debug模式，为false时禁用debug模式
 */

void FSpiMsgSetDebug(FSpiMsgCtrl *spi_msg_ctrl, boolean enable)
{
    u32 reg;
    reg = FSpiMsgReadRegfile(spi_msg_ctrl, FSPI_MSG_REGFILE_DEBUG);

    if (enable)
    {
        reg |= FSPI_MSG_REGFILE_DEBUG_VAL;
    }
    else
    {
        reg &= ~FSPI_MSG_REGFILE_DEBUG_VAL;
    }

    FSpiMsgWriteRegfile(spi_msg_ctrl, FSPI_MSG_REGFILE_DEBUG, reg);
}

/**
 * @name: FSpiMsgSetAlive
 * @msg: 启用或禁用spi msg Alive模式
 * @return {none}
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针， enable为true时启用Alive模式，为false时禁用Alive模式
 */

void FSpiMsgSetAlive(FSpiMsgCtrl *spi_msg_ctrl, boolean enable)
{
    u32 reg;
    reg = FSpiMsgReadRegfile(spi_msg_ctrl, FSPI_MSG_REGFILE_DEBUG);

    if (enable)
    {
        reg |= FSPI_MSG_REGFILE_ALIVE_VAL;
    }
    else
    {
        reg &= ~FSPI_MSG_REGFILE_ALIVE_VAL;
    }

    FSpiMsgWriteRegfile(spi_msg_ctrl, FSPI_MSG_REGFILE_DEBUG, reg);
}

/**
 * @name: FSpiMsgPrintStatus
 * @msg: 打印Message消息传输返回的状态
 * @return {int}，成功返回0，失败返回错误码
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 * @param {u8} status0, 协议命令状态
 * @param {u8} status1, 某些函数执行后的返回值
 */

int FSpiMsgPrintStatus(FSpiMsgCtrl *spi_msg_ctrl, u8 status0, u8 status1)
{
    if (status1 == ERR_SPI_SUCEESS)
    {
        return 0;
    }
    switch (status1)
    {
        case ERR_SPI_BUSY:
            FSPIM_MSG_ERROR("SPI bus is busy");
            break;
        case ERR_SPI_DMA_QUEUE:
            FSPIM_MSG_ERROR("DMA queue transfer error");
            break;
        case ERR_SPI_DMA_TIMEOUT:
            FSPIM_MSG_ERROR("DMA queue transfer timeout");
            break;
        case ERR_SPI_DEVT_TIMEOUT:
            FSPIM_MSG_ERROR("Operating flash timeout");
            break;
        case ERR_SPI_DMA_TXCHAN:
            FSPIM_MSG_ERROR("Spi_tx channel: DMA initialization error");
            break;
        case ERR_SPI_DMA_RXCHAN:
            FSPIM_MSG_ERROR("Spi_rx channel: DMA initialization error");
            break;
        case ERR_SPI_DMA_QUEUE_INIT_FAIL:
            FSPIM_MSG_ERROR("DMA queue initialization error");
            break;
        default:
            FSPIM_MSG_ERROR("status=0x%x, Unknown error", status1);
            break;
    }
    return -1;
}

/**
 * @name: FSpiMsgCheckResult
 * @msg: 确认命令执行结果
 * @return {int}，返回命令执行状态
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 */
int FSpiMsgCheckResult(FSpiMsgCtrl *spi_msg_ctrl)
{
    u32 timeout = 500000;
    FSpiMsgInfo *msg = (FSpiMsgInfo *)spi_msg_ctrl->msg;

    spi_msg_ctrl->cmd_completion = CMD_MSG_NOT_COMPLETION;

    /* wait for cmd msg completion */
    while (timeout--)
    {
        if (spi_msg_ctrl->cmd_completion == CMD_MSG_COMPLETION)
        {
            break;
        }
        FDriverUdelay(40);
    }

    if (timeout <= 0)
    {
        FSPIM_MSG_ERROR("FSpiMsgCheckResult timeout.");
        return -1;
    }

    FSPIM_MSG_INFO("cmd completion:%d", spi_msg_ctrl->cmd_completion);

    return FSpiMsgPrintStatus(spi_msg_ctrl, msg->status0, msg->status1);
}

void FSpiSetSubCmd(FSpiMsgCtrl *spi_msg_ctrl, u16 cmd_type, u16 sub_cmd)
{
    spi_msg_ctrl->msg->cmd_type = cmd_type;
    spi_msg_ctrl->msg->cmd_subid = sub_cmd;
}

/**
 * @name: FspiMsgSetDefault
 * @msg: 设置SPI控制器为默认状态
 * @return {int}，返回命令执行状态
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 */

int FspiMsgSetDefault(FSpiMsgCtrl *spi_msg_ctrl)
{
    int ret = 0;
    memset(spi_msg_ctrl->msg, 0, sizeof(FDevMsg));

    spi_msg_ctrl->msg->cmd_type = FMSG_MSG_CMD_DEFAULT;

    /* send interrupt to inform data comming */

    FSpiMsgWriteRegfile(spi_msg_ctrl, FMSG_M2S_INT_STATE, 0x10);
    /* wait to return status */
    ret = FSpiMsgCheckResult(spi_msg_ctrl);
    if (ret != 0)
    {
        FSPIM_MSG_ERROR("SpiSetDefault failed, ret = %x", ret);
    }
    return ret;
}


/**
 * @name: FSpiMsgSet
 * @msg: 传输Message消息
 * @return {int}，返回命令执行状态
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 */

int FSpiMsgSet(FSpiMsgCtrl *spi_msg_ctrl)
{
    int ret = 0;

    FSpiMsgShowMsg(spi_msg_ctrl->msg);

    FSpiMsgWriteRegfile(spi_msg_ctrl, FMSG_M2S_INT_STATE, 0x10);
    /* wait to return status */
    ret = FSpiMsgCheckResult(spi_msg_ctrl);

    if (ret != 0)
    {
        FSPIM_MSG_ERROR("FSpiMsgSet cmd_type=%x, sub_cmd=%x",
                        spi_msg_ctrl->msg->cmd_type, spi_msg_ctrl->msg->cmd_subid);
    }
    return ret;
}

/**
 * @name: FSpiMsgSetCmd8
 * @msg: 填充8位命令消息并发送
 * @return {int}，返回命令执行状态
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 * @param {u16}，sub_cmd, 子命令id
 * @param {u8}，cmd_data, 8位命令数据
 */

int FSpiMsgSetCmd8(FSpiMsgCtrl *spi_msg_ctrl, u16 sub_cmd, u8 cmd_data)
{
    int ret = 0;
    memset(spi_msg_ctrl->msg, 0, sizeof(FSpiMsgInfo));
    FSpiSetSubCmd(spi_msg_ctrl, FMSG_CMD_SET, sub_cmd);
    spi_msg_ctrl->msg->para[0] = cmd_data;
    /* send interrupt to inform data comming */
    FSpiMsgWriteRegfile(spi_msg_ctrl, FMSG_M2S_INT_STATE, 0x10);
    /* wait to return status */
    ret = FSpiMsgCheckResult(spi_msg_ctrl);
    return ret;
}

/**
 * @name: FSpiMsgSetCmd16
 * @msg: 填充8位命令消息并发送
 * @return {int}，返回命令执行状态
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 * @param {u16}，sub_cmd, 子命令id
 * @param {u8}，cmd_data, 16位命令数据
 */

int FSpiMsgSetCmd16(FSpiMsgCtrl *spi_msg_ctrl, u16 sub_cmd, u16 cmd_data)
{
    int ret = 0;
    memset(spi_msg_ctrl->msg, 0, sizeof(FSpiMsgInfo));

    u16 *cp_data = (u16 *)&spi_msg_ctrl->msg->para[0];

    memset(spi_msg_ctrl->msg, 0, sizeof(FSpiMsgInfo));
    FSpiSetSubCmd(spi_msg_ctrl, FMSG_CMD_SET, sub_cmd);

    *cp_data = cmd_data;

    /* send interrupt to inform data comming */
    FSpiMsgWriteRegfile(spi_msg_ctrl, FMSG_M2S_INT_STATE, 0x10);
    /* wait to return status */
    ret = FSpiMsgCheckResult(spi_msg_ctrl);
    return ret;
}

/**
 * @name: FSpiMsgSetCmd32
 * @msg: 填充8位命令消息并发送
 * @return {int}，返回命令执行状态
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 * @param {u16}，sub_cmd, 子命令id
 * @param {u8}，cmd_data, 32位命令数据
 */

int FSpiMsgSetCmd32(FSpiMsgCtrl *spi_msg_ctrl, u16 sub_cmd, u32 cmd_data)
{
    int ret = 0;

    u32 *cp_data = (u32 *)&spi_msg_ctrl->msg->para[0];

    memset(spi_msg_ctrl->msg, 0, sizeof(FDevMsg));
    FSpiSetSubCmd(spi_msg_ctrl, FMSG_CMD_SET, sub_cmd);

    *cp_data = cmd_data;

    /* send interrupt to inform data comming */
    FSpiMsgWriteRegfile(spi_msg_ctrl, FMSG_M2S_INT_STATE, 0x10);
    /* wait to return status */
    ret = FSpiMsgCheckResult(spi_msg_ctrl);

    return ret;
}

/**
 * @name: FSpiMsgSetClkDiv
 * @msg: 设置SPI控制器分频系数
 * @return {int}，返回命令执行状态
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 * @param {u32}，clk_div, 分频系数值
 */

int FSpiMsgSetClkDiv(FSpiMsgCtrl *spi_msg_ctrl, u32 clk_div)
{
    int ret = 0;
    ret = FSpiMsgSetCmd32(spi_msg_ctrl, FSPI_MSG_SUBCMD_SET_BAUDR, clk_div);
    if (ret != 0)
    {
        FSPIM_MSG_ERROR("SetClkDiv failed.");
    }
    return ret;
}

/**
 * @name: FSpiMsgSetClkDiv
 * @msg: 设置SPI控制器传输频率，spi传输速率(clk_div) = 最大频率(max_freq) / 分频系数(clk_div), clk_div必须为2~65534之间的偶数
 * @return {int}，返回命令执行状态
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 * @param {u32}clk_freq, 传输频率
 */

int FSpiMsgSetSpeed(FSpiMsgCtrl *spi_msg_ctrl, u32 clk_freq)
{
    u32 clk_div; /* 分频系数 */

    int ret = 0;

    if (clk_freq == 0)
    {
        FSPIM_MSG_ERROR("Input spi clock frequency is %d => do not support, this "
                        "parameter should not be 0.",
                        clk_freq);
        return -1;
    }
    else
    {
        clk_div = FSPI_CLK_FREQ_HZ / clk_freq;
    }

    if ((clk_div >= FSPIM_BAUD_R_SCKDV_MIN) && (clk_div <= FSPIM_BAUD_R_SCKDV_MAX))
    {
        if ((clk_div % 2) != 0)
        {
            clk_div += 1; /* 如果不是偶数则替换为一个大于目前值的最小的偶数 */
        }

        FSpiMsgSetEnable(spi_msg_ctrl, FALSE);
        FSPIM_MSG_INFO("Set spi clock divider as %d", clk_div);
        ret = FSpiMsgSetClkDiv(spi_msg_ctrl, clk_div);
        FSpiMsgSetEnable(spi_msg_ctrl, TRUE);
        return ret;
    }
    else
    {
        FSPIM_MSG_ERROR("Input spi clock frequency is %ld, this parameter be set "
                        "wrong. spi clock divider = %d, this parameter should be an "
                        "even from 2 to 65534.",
                        clk_freq, clk_div);
        return -1;
    }
}

/**
 * @name: FSpiMsgSetDataWidth
 * @msg: 设置spi发送的数据宽度
 * @return {int}，返回命令执行状态
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 * @param {u32}n_bytes, 1: 1字节，8位宽度；2：2字节，16位宽度
 */

int FSpiMsgSetDataWidth(FSpiMsgCtrl *spi_msg_ctrl, u8 n_bytes)
{
    int ret = 0;
    u8 data_width = (n_bytes == 1) ? 8 : 16;
    ret = FSpiMsgSetCmd8(spi_msg_ctrl, FSPI_MSG_SUBCMD_SET_DATA_WIDTH, data_width);
    if (ret != 0)
    {
        FSPIM_MSG_ERROR("SetDataWidth failed.");
    }
    return ret;
}

/**
 * @name: FSpiMsgSetMode
 * @msg: 设置spi控制器的模式
 * @return {int}，返回命令执行状态
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 * @param {FSpimCphaCpolMode}，bit[0]: cpha , bit[1]: cpol
 */

int FSpiMsgSetMode(FSpiMsgCtrl *spi_msg_ctrl, FSpimCphaCpolMode mode)
{
    int ret = 0;
    u8 cpha, cpol;
    u16 mode_val;
    cpha = mode & 0x1;
    cpol = mode >> 1;
    mode_val = (cpha << 8) | cpol;
    /* mode_val: bit[0:7]: cpol, bit[8:15]: cpha*/
    ret = FSpiMsgSetCmd16(spi_msg_ctrl, FSPI_MSG_SUBCMD_SET_MODE, (u16)mode_val);
    if (ret != 0)
    {
        FSPIM_MSG_ERROR("SetMode failed.");
    }
    return ret;
}

/**
 * @name: FSpiMsgSetTransMode
 * @msg: 设置SPI传输模式
 * @return {int} 成功返回0，失败返回非零错误码
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 * @param {u32} tmod, SPI传输模式设置：rx_only, tx_only, rx_tx
 */

int FSpiMsgSetTransMode(FSpiMsgCtrl *spi_msg_ctrl, u8 tmod)
{
    int ret = 0;
    ret = FSpiMsgSetCmd8(spi_msg_ctrl, FSPI_MSG_SUBCMD_SET_TMOD, tmod);
    if (ret != 0)
    {
        FSPIM_MSG_ERROR("SetTmode failed.");
    }
    return ret;
}


/**
 * @name: FSpiMsgMaskIntr
 * @msg: 设置中断屏蔽
 * @return {int} 成功返回0，失败返回非零错误码
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 * @param {u32} enable, 1:使能中断，0: 屏蔽中断
 */
int FSpiMsgMaskIntr(FSpiMsgCtrl *spi_msg_ctrl, u8 enable)
{
    return FSpiMsgSetCmd8(spi_msg_ctrl, FSPI_MSG_SUBCMD_SET_INTERRUPT, enable);
}

/**
 * @name: FSpiMsgSetEnable
 * @msg: 开启或关闭SPI控制器
 * @return {int} 成功返回0，失败返回非零错误码
 * @param {FSpiMsgCtrl *}，spi控制器结构体指针
 * @param {u32} enable, TRUE:使能SPI控制器，FALSE: 禁用SPI控制器
 */

int FSpiMsgSetEnable(FSpiMsgCtrl *spi_msg_ctrl, u8 enable)
{
    u8 val = enable ? 1 : 2;
    int ret = 0;
    ret = FSpiMsgSetCmd8(spi_msg_ctrl, FSPI_MSG_SUBCMD_SET_MODULE_EN, val);
    if (ret != 0)
    {
        FSPIM_MSG_ERROR("SetEnableChip failed.");
    }
    return ret;
}

/**
 * @name: FSpiMsgSetChipSelection
 * @msg: 设置片选信号
 * @return {NONE}
 * @param {FSpiMsgCtrl *} ，spi控制器结构体指针
 * @param {boolean} enable, TRUE: 片选打开, FALSE: 片选关闭
 */
int FSpiMsgSetChipSelection(FSpiMsgCtrl *spi_msg_ctrl, boolean enable)
{
    FSpiMsgSlaveDevice cs_n = spi_msg_ctrl->spi_msg_config.slave_dev_id;
    u32 origin;

    int ret = 0;

    if (!enable)
    {
        origin = FSPI_MSG_CHIP_CS_EN(cs_n) | FSPI_MSG_CHIP_SEL(cs_n);
        cs_n = (0x1 << 8) | origin;
        ret = FSpiMsgSetCmd16(spi_msg_ctrl, FSPI_MSG_SUBCMD_SET_CS, cs_n);
    }
    else
    {
        origin = FSPI_MSG_CHIP_CS_EN(cs_n) & !FSPI_MSG_CHIP_SEL(cs_n);
        cs_n = (0x1 << 8) | origin;
        ret = FSpiMsgSetCmd16(spi_msg_ctrl, FSPI_MSG_SUBCMD_SET_CS, cs_n);
    }
    return ret;
}
