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
 *
 * FilePath: fspim_hw.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 09:08:00
 * Description:  This file is for providing spim Hardware interaction func.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/12/3   init commit
 * 1.1   zhugengyu  2022/4/15   support test mode
 * 1.2  liqiaozhong 2023/1/4    add data get func
 */


#include "fassert.h"
#include "fdrivers_port.h"
#include "fspim_hw.h"
#include "fspim.h"

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FSPIM_DEBUG_TAG "SPIM-HW"
#define FSPIM_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_WARN(format, ...) FT_DEBUG_PRINT_W(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_INFO(format, ...) FT_DEBUG_PRINT_I(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/**
 * @name: FSpimGetTxFifoDepth
 * @msg: 获取TX Fifo可以设置的最大深度
 * @return {u32} TX Fifo的深度
 * @param {uintptr} base_addr, SPI控制器基地址
 */
u32 FSpimGetTxFifoDepth(uintptr base_addr)
{
    u32 fifo_depth;
    for (fifo_depth = 1; fifo_depth < FSPIM_MAX_FIFO_DEPTH; fifo_depth++)
    {
        FSpimSetTxFifoThreshold(base_addr, fifo_depth); /* 不断尝试设置一个更大的值，直到不能再设置 */
        if (fifo_depth != FSpimGetTxFifoThreshold(base_addr))
        {
            FSPIM_INFO("The Tx fifo threshold is %d", fifo_depth);
            break;
        }
    }

    FSpimSetTxFifoThreshold(base_addr, 0);
    return fifo_depth;
}

/**
 * @name: FSpimGetRxFifoDepth
 * @msg: 获取RX Fifo可以设置的最大深度
 * @return {u32} Rx Fifo的深度
 * @param {uintptr} base_addr, SPI控制器基地址
 */
u32 FSpimGetRxFifoDepth(uintptr base_addr)
{
    u32 fifo_depth = FSPIM_MIN_FIFO_DEPTH;
    while (FSPIM_MAX_FIFO_DEPTH >= fifo_depth)
    {
        FSpimSetRxFifoThreshold(base_addr, fifo_depth);
        if (fifo_depth != FSpimGetRxFifoThreshold(base_addr))
        {
            FSPIM_INFO("The Rx fifo threshold is %d", fifo_depth);
            break;
        }

        fifo_depth++;
    }

    return fifo_depth;
}

/**
 * @name: FSpimSelSlaveDev
 * @msg: 选择SPI从设备
 * @return {无}
 * @param {uintptr} base_addr, SPI控制器基地址
 * @param {u32} slave_dev_id, 从设备ID
 */
void FSpimSelSlaveDev(uintptr base_addr, u32 slave_dev_id)
{
    FASSERT(slave_dev_id < FSPIM_NUM_OF_SLAVE_DEV);
    u32 reg_val;

    reg_val = (FSPIM_SER_SELECT << slave_dev_id);
    FSPIM_WRITE_REG32(base_addr, FSPIM_SER_OFFSET, reg_val);

    return;
}

/**
 * @name: FSpimSetSpeed
 * @msg: 设置SPI传输速度
 * @return {FError}
 * @param {uintptr} base_addr, SPI控制器基地址
 * @param {u32} clk_div, SPI期望配置的频率
 */
FError FSpimSetSpeed(uintptr base_addr, u32 clk_freq)
{
    u32 clk_div; /* 分频系数 */

    if (clk_freq == 0)
    {
        FSPIM_ERROR("Input spi clock frequency is %d => do not support, this parameter "
                    "should not be 0.",
                    clk_freq);
        return FSPIM_ERR_NOT_SUPPORT;
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

        FSpimSetEnable(base_addr, FALSE);
        FSPIM_INFO("Set spi clock divider as %d", clk_div);
        FSPIM_WRITE_REG32(base_addr, FSPIM_BAUD_R_OFFSET, clk_div);
        FSpimSetEnable(base_addr, TRUE);
        return FSPIM_SUCCESS;
    }
    else
    {
        FSPIM_ERROR("Input spi clock frequency is %ld, this parameter be set wrong. "
                    "spi clock divider = %d, this parameter should be an even from 2 "
                    "to 65534.",
                    clk_freq, clk_div);
        return FSPIM_ERR_NOT_SUPPORT;
    }
}

/**
 * @name: FSpimGetSpeed
 * @msg: 获取SPI传输速度
 * @return {u32}FSPIM传输频率
 * @param {uintptr} base_addr, SPI控制器基地址
 */
u32 FSpimGetSpeed(uintptr base_addr)
{
    return FSPIM_READ_REG32(base_addr, FSPIM_BAUD_R_OFFSET);
}

/**
 * @name: FSpimSetTransMode
 * @msg: 设置SPI传输模式
 * @return {无}
 * @param {uintptr} base_addr, SPI控制器基地址
 * @param {u32} trans_mode, SPI传输模式设置
 */
void FSpimSetTransMode(uintptr base_addr, u32 trans_mode)
{
    FASSERT(trans_mode < FSPIM_TRANS_MODE_MAX);
    u32 reg_val;
    boolean enabled = FSpimGetEnable(base_addr);

    if (enabled)
    {
        FSpimSetEnable(base_addr, FALSE);
    }

    reg_val = FSpimGetCtrlR0(base_addr);
    reg_val &= ~FSPIM_CTRL_R0_TMOD_MASK; /* clear trans mode bits */
    switch (trans_mode)
    {
        case FSPIM_TRANS_MODE_RX_TX:
            reg_val |= FSPIM_CTRL_R0_TMOD(FSPIM_TMOD_RX_TX);
            break;
        case FSPIM_TRANS_MODE_TX_ONLY:
            reg_val |= FSPIM_CTRL_R0_TMOD(FSPIM_TMOD_TX_ONLY);
            break;
        case FSPIM_TRANS_MODE_RX_ONLY:
            reg_val |= FSPIM_CTRL_R0_TMOD(FSPIM_TMOD_RX_ONLY);
            break;
        case FSPIM_TRANS_MODE_READ_EEPROM:
            reg_val |= FSPIM_CTRL_R0_TMOD(FSPIM_TMOD_RD_EEPROM);
            break;
        default:
            FASSERT(0);
            break;
    }

    FSpimSetCtrlR0(base_addr, reg_val);

    if (enabled)
    {
        FSpimSetEnable(base_addr, TRUE);
    }

    return;
}

/**
 * @name: FSpimSetCpha
 * @msg: 设置串行时钟相位
 * @return {无}
 * @param {uintptr} base_addr, SPI控制器基地址
 * @param {u32} cpha_mode, SPI控制器的相位设置
 */
void FSpimSetCpha(uintptr base_addr, u32 cpha_mode)
{
    u32 reg_val = FSpimGetCtrlR0(base_addr);

    reg_val &= ~FSPIM_CTRL_R0_SCPHA_MASK; /* clear bits */
    if (FSPIM_CPHA_1_EDGE == cpha_mode)
    {
        reg_val |= FSPIM_CTRL_R0_SCPHA(FSPIM_SCPHA_SWITCH_DATA_MID);
    }
    else if (FSPIM_CPHA_2_EDGE == cpha_mode)
    {
        reg_val |= FSPIM_CTRL_R0_SCPHA(FSPIM_SCPHA_SWITCH_DATA_BEG);
    }
    else
    {
        FASSERT(0);
    }

    FSpimSetCtrlR0(base_addr, reg_val);
}

/**
 * @name: FSpimGetCpha
 * @msg: 获取串行时钟相位
 * @return {FSpimCphaType}串行时钟相位
 * @param {uintptr} base_addr, SPI控制器基地址
 */
FSpimCphaType FSpimGetCpha(uintptr base_addr)
{
    u32 reg_val = FSpimGetCtrlR0(base_addr);

    if (reg_val &= FSPIM_CTRL_R0_SCPHA(FSPIM_SCPHA_SWITCH_DATA_BEG))
    {
        return FSPIM_CPHA_2_EDGE;
    }
    else
    {
        return FSPIM_CPHA_1_EDGE;
    }
}

/**
 * @name: FSpimSetCpol
 * @msg: 设置串行时钟极性
 * @return {无}
 * @param {uintptr} base_addr, SPI控制器基地址
 * @param {u32} cpol_mode, SPI控制器的极性设置
 */
void FSpimSetCpol(uintptr base_addr, u32 cpol_mode)
{
    u32 reg_val = FSpimGetCtrlR0(base_addr);

    reg_val &= ~FSPIM_CTRL_R0_SCPOL_MASK; /* clear bits */
    if (FSPIM_CPOL_LOW == cpol_mode)
    {
        reg_val |= FSPIM_CTRL_R0_SCPOL(FSPIM_SCPOL_INACTIVE_LOW);
    }
    else if (FSPIM_CPOL_HIGH == cpol_mode)
    {
        reg_val |= FSPIM_CTRL_R0_SCPOL(FSPIM_SCPOL_INACTIVE_HIGH);
    }
    else
    {
        FASSERT(0);
    }

    FSpimSetCtrlR0(base_addr, reg_val);
}

/**
 * @name: FSpimGetCpol
 * @msg: 获取串行时钟极性
 * @return {无}
 * @param {uintptr} base_addr, SPI控制器基地址
 */
FSpimCpolType FSpimGetCpol(uintptr base_addr)
{
    u32 reg_val = FSpimGetCtrlR0(base_addr);

    if (reg_val &= FSPIM_CTRL_R0_SCPOL(FSPIM_SCPOL_INACTIVE_HIGH))
    {
        return FSPIM_CPOL_HIGH;
    }
    else
    {
        return FSPIM_CPOL_LOW;
    }
}

/**
 * @name: FSpimSetSlaveEnable
 * @msg: 使能/去使能和从设备的连接
 * @return {无}
 * @param {uintptr} base_addr, SPI控制器基地址
 * @param {boolean} enable, TRUE: 使能从设备, FALSE: 去使能从设备
 */
void FSpimSetSlaveEnable(uintptr base_addr, boolean enable)
{
    u32 reg_val;
    boolean enabled = FSpimGetEnable(base_addr);

    if (enabled)
    {
        FSpimSetEnable(base_addr, FALSE);
    }

    reg_val = FSpimGetCtrlR0(base_addr);

    reg_val &= ~FSPIM_CTRL_R0_SLV_OE_MASK;
    if (enable)
    {
        reg_val |= FSPIM_CTRL_R0_SLV_OE(FSPIM_SLAVE_TX_ENABLE);
    }
    else
    {
        reg_val |= FSPIM_CTRL_R0_SLV_OE(FSPIM_SLAVE_TX_DISALE);
    }

    FSpimSetCtrlR0(base_addr, reg_val);

    if (enabled)
    {
        FSpimSetEnable(base_addr, TRUE);
    }

    return;
}