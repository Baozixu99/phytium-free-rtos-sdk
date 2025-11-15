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
 * FilePath: fi2c_hw.c
 * Date: 2021-11-01 14:53:42
 * LastEditTime: 2022-02-18 08:36:22
 * Description:  This file is for I2C register read/write operation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhugengyu 2021/11/1  first commit
 * 1.1  liushengming 2022/02/18  support slave mode
 */

/***************************** Include Files *********************************/
#include <string.h>
#include "fdrivers_port.h"
#include "ferror_code.h"
#include "fassert.h"
#include "fi2c_hw.h"
#include "fi2c.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FI2C_DEBUG_TAG          "I2C_HW"
#define FI2C_ERROR(format, ...) FT_DEBUG_PRINT_E(FI2C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_INFO(format, ...)  FT_DEBUG_PRINT_I(FI2C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_DEBUG(format, ...) FT_DEBUG_PRINT_D(FI2C_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/**
 * @name: FI2cClearIntrBits
 * @msg: 清除中断状态位，返回清除前的中断状态
 * @return {*}
 * @param {uintptr} uintptr, I2C控制器地址
 * @param {u32} *last_err_p, Abort错误
 */
u32 FI2cClearIntrBits(uintptr addr, u32 *last_err_p)
{
    FASSERT(last_err_p);

    const u32 stat = FI2C_READ_INTR_STAT(addr);

    /* read to clr interrupt status bits */
    if (stat & FI2C_INTR_TX_ABRT)
    {
        *last_err_p = FI2C_READ_REG32(addr, FI2C_TX_ABRT_SOURCE_OFFSET); /* read out abort sources */
        FI2C_READ_REG32(addr, FI2C_CLR_TX_ABRT_OFFSET);
    }

    if (stat & FI2C_INTR_RX_UNDER)
    {
        FI2C_READ_REG32(addr, FI2C_CLR_RX_UNDER_OFFSET);
    }

    if (stat & FI2C_INTR_RX_OVER)
    {
        FI2C_READ_REG32(addr, FI2C_CLR_RX_OVER_OFFSET);
    }

    if (stat & FI2C_INTR_TX_OVER)
    {
        FI2C_READ_REG32(addr, FI2C_CLR_TX_OVER_OFFSET);
    }

    if (stat & FI2C_INTR_RX_DONE)
    {
        FI2C_READ_REG32(addr, FI2C_CLR_RX_DONE_OFFSET);
    }

    if (stat & FI2C_INTR_ACTIVITY)
    {
        FI2C_READ_REG32(addr, FI2C_CLR_ACTIVITY_OFFSET);
    }

    if (stat & FI2C_INTR_STOP_DET)
    {
        FI2C_READ_REG32(addr, FI2C_CLR_STOP_DET_OFFSET);
    }

    if (stat & FI2C_INTR_START_DET)
    {
        FI2C_READ_REG32(addr, FI2C_CLR_START_DET_OFFSET);
    }

    if (stat & FI2C_INTR_GEN_CALL)
    {
        FI2C_READ_REG32(addr, FI2C_CLR_GEN_CALL_OFFSET);
    }

    return stat;
}

/**
 * @name: FI2cClearAbort
 * @msg: 清除异常状态位，返回清除前的中断状态
 * @return {*}
 * @param {uintptr} addr, I2C控制器地址
 */
void FI2cClearAbort(uintptr addr)
{
    u32 timeout = FI2C_TIMEOUT;

    do
    {
        FI2C_CLEAR_INTR_STATUS(addr);
        if ((FI2C_READ_REG32(addr, FI2C_TX_ABRT_SOURCE_OFFSET)) == 0)
        {
            return;
        }

    } while (0 != timeout--);
    FI2C_ERROR("Timeout when clear abort.");
    return;
}


/**
 * @name: FI2cWaitStatus
 * @msg: 等待特定的I2C状态位直到状态不存在或者超时
 * @return {*}
 * @param {uintptr} addr, I2C控制器地址
 * @param {u32} stat_bit, I2C状态位
 */
FError FI2cWaitStatus(uintptr addr, u32 stat_bit)
{
    u32 timeout = 0;

    /* wait until statbit was set or timeout */
    while (!((FI2C_READ_REG32(addr, FI2C_STATUS_OFFSET)) & stat_bit) && (FI2C_TIMEOUT > ++timeout))
    {
        FDriverUdelay(1); /*wait 1 us*/
    }

    if (FI2C_TIMEOUT <= timeout)
    {
        FI2C_ERROR("Timeout when wait status: 0x%x.", stat_bit);
        return FI2C_ERR_TIMEOUT;
    }

    return FI2C_SUCCESS;
}

/**
 * @name: FI2cWaitBusBusy
 * @msg: 等待I2C总线忙
 * @return {*}
 * @param {uintptr} addr, I2C控制器地址
 */
FError FI2cWaitBusBusy(uintptr addr)
{
    u32 ret = FI2C_SUCCESS;

    if (((FI2C_READ_REG32(addr, FI2C_STATUS_OFFSET)) & FI2C_STATUS_MST_ACTIVITY) &&
        (FI2C_SUCCESS != FI2cWaitStatus(addr, FI2C_STATUS_TFE)))
    {
        ret = FI2C_ERR_TIMEOUT;
        FI2C_ERROR("Timeout when wait i2c bus busy.");
    }

    return ret;
}

/**
 * @name: FI2cFlushRxFifo
 * @msg: 等待接收Fifo传输完成
 * @return {*}
 * @param {FI2c} *instance_p, I2C控制器实例
 */
FError FI2cFlushRxFifo(uintptr addr)
{
    int timeout = 0;
    FError ret = FI2C_SUCCESS;

    /* read data to trigger trans until fifo empty */
    while (FI2C_GET_STATUS(addr) & FI2C_STATUS_RFNE)
    {
        (void)FI2C_READ_DATA(addr);

        if (FI2C_TIMEOUT < ++timeout)
        {
            ret = FI2C_TIMEOUT;
            FI2C_ERROR("Timeout when flush fifo.");
            break;
        }
    }

    return ret;
}

/**
 * @name: FI2cSetEnable
 * @msg: 设置I2C控制器的使能状态
 * @return {*}
 * @param {uintptr} addr, I2c控制器地址
 * @param {boolean} enable， TRUE: 使能，FALSE: 去使能
 */
FError FI2cSetEnable(uintptr addr, boolean enable)
{
    u32 status = enable ? FI2C_IC_ENABLE : FI2C_IC_DISABLE;
    u32 timeout = FI2C_TIMEOUT;

    do
    {
        FI2C_WRITE_REG32(addr, FI2C_ENABLE_OFFSET, status);
        if (((FI2C_READ_REG32(addr, FI2C_ENABLE_STATUS_OFFSET)) & FI2C_IC_ENABLE_MASK) == status)
        {
            return FI2C_SUCCESS;
        }

    } while (0 != timeout--);

    FI2C_ERROR("Timeout in %sabling I2C ctrl.", enable ? "en" : "dis");
    return FI2C_ERR_TIMEOUT;
}

/**
 * @name: FI2cCtrlDisable
 * @msg:  关闭I2C控制器
 * @return {*}
 * @param {FI2c} *instance_p, I2C驱动实例数据
 */
void FI2cCtrlDisable(uintptr addr)
{
    FI2C_WRITE_REG32(addr, FI2C_RX_TL_OFFSET, 0);
    FI2C_WRITE_REG32(addr, FI2C_TX_TL_OFFSET, 0);
    FI2C_SET_INTERRUPT_MASK(addr, 0); /* disable all intr */

    FI2cSetEnable(addr, FALSE); /* disable i2c ctrl */

    return;
}