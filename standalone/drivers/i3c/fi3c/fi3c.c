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
 * FilePath: fi3c.c
 * Date: 2021-11-01 14:53:42
 * LastEditTime: 2022-02-18 08:36:58
 * Description:  This file is for complete user external interface
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhangyan    2023/9/11  first commit
 */
#include <string.h>
#include "fio.h"
#include "ferror_code.h"
#include "ftypes.h"
#include "fdebug.h"
#include "fi3c_hw.h"
#include "fi3c.h"

#define FI3C_DEBUG_TAG          "I3C_C"
#define FI3C_ERROR(format, ...) FT_DEBUG_PRINT_E(FI3C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI3C_INFO(format, ...)  FT_DEBUG_PRINT_I(FI3C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI3C_DEBUG(format, ...) FT_DEBUG_PRINT_D(FI3C_DEBUG_TAG, format, ##__VA_ARGS__)
/**
 * @name: FI3cCfgInitialize
 * @msg: 完成I3C驱动实例的初始化，使之可以使用
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {FI3cConfig} *cofig_p I3C驱动配置数据
 * @return SUCCESS if initialization was successful
 *         ERROR
 */
FError FI3cCfgInitialize(FI3c *instance_p, const FI3cConfig *input_config_p)
{
    FASSERT(instance_p && input_config_p);

    FError ret = FI3C_SUCCESS;

    /*
        * If the device is started, disallow the initialize and return a Status
        * indicating it is started.  This allows the user to de-initialize the device
        * and reinitialize, but prevents a user from inadvertently
        * initializing.
    */
    if (FT_COMPONENT_IS_READY == instance_p->is_ready)
    {
        FI3C_ERROR("Device is already initialized!!!");
        return FI3C_ERR_INVAL_STATE;
    }

    /*
        * Set default values and configuration data, including setting the
        * callback handlers to stubs  so the system will not crash should the
        * application not assign its own callbacks.
     */
    FI3cDeInitialize(instance_p);
    instance_p->config = *input_config_p;

    /*
    * Reset the device.
    */
    ret = FI3cReset(instance_p);
    if (FI3C_SUCCESS == ret)
    {
        instance_p->is_ready = FT_COMPONENT_IS_READY;
    }

    return ret;
}

/**
 * @name: FI3cDeInitialize
 * @msg: 完成I3C驱动实例去使能，清零实例数据
 * @return {*}
 * @param {FI3c} *instance_p
 */
void FI3cDeInitialize(FI3c *instance_p)
{
    FASSERT(instance_p);
    uintptr base_addr = instance_p->config.base_addr;
    FI3C_WRITE_REG32(base_addr, FI3C_DEVS_CTRL_OFFSET, FI3C_DEV_CLR_ALL);
    instance_p->is_ready = 0;
    memset(instance_p, 0, sizeof(*instance_p));
}

/**
 * @name: FI3cReset
 * @msg: 完成I3C设备配置
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p
 */
FError FI3cReset(FI3c *instance_p)
{
    FASSERT(instance_p);
    FError ret = FI3C_SUCCESS;
    u32 prescl0_reg, scl_clk_i3c, scl_clk_i2c, scl_clk_freq;
    uintptr base_addr = instance_p->config.base_addr;

    scl_clk_freq = FI3C_CLK_FREQ_HZ;
    if (instance_p->config.trans_mode == FI3C_MODE)
    {
        scl_clk_i3c = ((scl_clk_freq / (instance_p->config.i3c_sclrate * 4)) - 1);
        prescl0_reg = FI3C_READ_REG32(base_addr, FI3C_PRESCL_CTRL0_OFFSET) &
                      (~FI3C_PRESCL_CTRL0_I3C_MASK);
        prescl0_reg |= FI3C_PRESCL_CTRL0_I3C(scl_clk_i3c);
    }
    else
    {
        scl_clk_i2c = ((scl_clk_freq / (instance_p->config.i2c_sclrate * 5)) - 1);
        prescl0_reg = FI3C_READ_REG32(base_addr, FI3C_PRESCL_CTRL0_OFFSET) &
                      (~FI3C_PRESCL_CTRL0_I2C_MASK);
        prescl0_reg |= FI3C_PRESCL_CTRL0_I2C(scl_clk_i2c);
    }

    FI3C_WRITE_REG32(base_addr, FI3C_PRESCL_CTRL0_OFFSET, prescl0_reg);
    FI3C_DEBUG("FI3C_PRESCL_CTRL0 = %x", FI3C_READ_REG32(base_addr, FI3C_PRESCL_CTRL0_OFFSET));
    FI3C_WRITE_REG32(base_addr, FI3C_DEVS_CTRL_OFFSET, FI3C_DEV_CLR_ALL);

    return ret;
}

/**
 * @name: FI3cInterruptEnable
 * @msg: 使能I3C中断
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p
 * @param {FI3cIntrEventType} *event_type具体中断类型
 */
FError FI3cInterruptEnable(FI3c *instance_p, FI3cIntrEventType event_type)
{
    FASSERT(instance_p != NULL);
    FASSERT(FT_COMPONENT_IS_READY == instance_p->is_ready);

    uintptr base_addr = instance_p->config.base_addr;
    u32 reg_val = 0;
    reg_val = FI3C_READ_REG32(base_addr, FI3C_MST_IER_OFFSET);
    switch (event_type)
    {
        case FI3C_INTR_EVENT_CMDD_EMP:
            reg_val |= FI3C_MST_INT_CMDD_EMP_MASK;
            break;
        default:
            break;
    }

    FI3C_WRITE_REG32(base_addr, FI3C_MST_IER_OFFSET, reg_val);

    return FI3C_SUCCESS;
}