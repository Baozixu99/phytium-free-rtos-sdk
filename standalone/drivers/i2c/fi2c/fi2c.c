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
 * FilePath: fi2c.c
 * Date: 2021-11-01 14:53:42
 * LastEditTime: 2022-02-18 08:36:58
 * Description:  This file is for complete user external interface
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhugengyu 2021/11/1  first commit
 * 1.1  liushengming 2022/2/18  modified to support i2c
 */


/***************************** Include Files *********************************/
#include <string.h>
#include "fio.h"
#include "ferror_code.h"
#include "ftypes.h"
#include "fdrivers_port.h"
#include "fi2c_hw.h"
#include "fi2c.h"

/************************** Constant Definitions *****************************/

typedef struct
{
    u32 speed_mode;
    u32 scl_lcnt;
    u32 scl_hcnt;
    u32 sda_hold;
} FI2cSpeedCfg; /* speed related configs */

typedef struct
{
    u32 min_scl_hightime_ns;
    u32 min_scl_lowtime_ns;
    u32 def_risetime_ns;
    u32 def_falltime_ns;
} FI2cSpeedModeInfo; /* speed calculation related configs */

/************************** Variable Definitions *****************************/
// clang-format off
static const FI2cSpeedModeInfo i2c_speed_cfg[FI2C_SPEED_MODE_MAX] =
{
    [FI2C_STANDARD_SPEED] = {
        4000,
        4700,
        1000,
        300,
    },
    [FI2C_FAST_SPEED] = {
        600,
        1300,
        300,
        300,
    },
    [FI2C_HIGH_SPEED] = {
        390,
        460,
        60,
        160,
    }
};/* i2c 速率配置时序参数，此参数为i2c 协议建议值，具体配置参数可调 */

static const FI2cSpeedCfg i2c_speed_reg_cfg[FI2C_SPEED_MODE_MAX] =
{
    [FI2C_STANDARD_SPEED] = {
        FI2C_STANDARD_SPEED,
        232,
        205,
        15,
    },
    [FI2C_FAST_SPEED] = {
        FI2C_FAST_SPEED,
        79,
        18,
        15,
    },
    [FI2C_HIGH_SPEED] = {
        FI2C_HIGH_SPEED,
        31,
        6,
        15,
    }
};/*i2c 速率配置寄存器值，具体配置参数可调 ：SCL_FREQ = I2C_CLK_FREQ / (SCL_L_CNT + SCL_H_CNT + 1 + 7 +I2C_SPKLEN) */
// clang-format on

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FI2C_DEBUG_TAG          "I2C"
#define FI2C_ERROR(format, ...) FT_DEBUG_PRINT_E(FI2C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_INFO(format, ...)  FT_DEBUG_PRINT_I(FI2C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_DEBUG(format, ...) FT_DEBUG_PRINT_D(FI2C_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/**
 * @name: FI2cCfgInitialize
 * @msg: 完成I2C驱动实例的初始化，使之可以使用
 * @param {FI2c} *instance_p I2C驱动实例数据
 * @param {FI2cConfig} *cofig_p I2C驱动配置数据
 * @return SUCCESS if initialization was successful
 *         ERROR
 */
FError FI2cCfgInitialize(FI2c *instance_p, const FI2cConfig *input_config_p)
{
    FASSERT(instance_p && input_config_p);

    FError ret = FI2C_SUCCESS;

    /*
        * If the device is started, disallow the initialize and return a Status
        * indicating it is started.  This allows the user to de-initialize the device
        * and reinitialize, but prevents a user from inadvertently
        * initializing.
    */
    if (FT_COMPONENT_IS_READY == instance_p->is_ready)
    {
        FI2C_ERROR("Device is already initialized!!!");
        return FI2C_ERR_INVAL_STATE;
    }

    /*
        * Set default values and configuration data, including setting the
        * callback handlers to stubs  so the system will not crash should the
        * application not assign its own callbacks.
     */
    FI2cDeInitialize(instance_p);
    instance_p->config = *input_config_p;

    if (FI2C_SUCCESS == ret)
    {
        instance_p->is_ready = FT_COMPONENT_IS_READY;
    }

    return ret;
}

/**
 * @name: FI2cDeInitialize
 * @msg: 完成I2C驱动实例去使能，清零实例数据
 * @return {*}
 * @param {FI2c} *instance_p
 */
void FI2cDeInitialize(FI2c *instance_p)
{
    FASSERT(instance_p);
    instance_p->is_ready = 0;

    memset(instance_p, 0, sizeof(*instance_p));
}

/**
 * @name: FI2cCalcTiming
 * @msg: 计算I2C的上升沿下降沿配置
 * @return {*}
 * @param {FI2c} *instance_p, I2C控制器实例
 * @param {u32} spk_cnt, spk数目
 * @param {FI2cSpeedCfg} *speed_cfg_p，速度配置
 */
static FError FI2cCalcTiming(FI2c *instance_p, u32 spk_cnt, FI2cSpeedCfg *speed_cfg_p)
{
    FASSERT(speed_cfg_p);
    FASSERT(instance_p);
    u32 speed_mode = speed_cfg_p->speed_mode;
    u32 bus_clk_hz = instance_p->config.ref_clk_hz;
    FError ret = FI2C_SUCCESS;
    const FI2cSpeedModeInfo *info_p = &i2c_speed_cfg[speed_mode];
    int fall_cnt, rise_cnt, min_t_low_cnt, min_t_high_cnt;
    int hcnt, lcnt, period_cnt, diff, tot;
    int scl_rise_time_ns, scl_fall_time_ns;

    period_cnt = bus_clk_hz / instance_p->config.speed_rate;
    scl_rise_time_ns = info_p->def_risetime_ns;
    scl_fall_time_ns = info_p->def_falltime_ns;

    /* convert a period to a number of IC clk cycles */
    rise_cnt = DIV_ROUND_UP(bus_clk_hz / 1000 * scl_rise_time_ns, NANO_TO_KILO);
    fall_cnt = DIV_ROUND_UP(bus_clk_hz / 1000 * scl_fall_time_ns, NANO_TO_KILO);
    min_t_low_cnt = DIV_ROUND_UP(bus_clk_hz / 1000 * info_p->min_scl_lowtime_ns, NANO_TO_KILO);
    min_t_high_cnt = DIV_ROUND_UP(bus_clk_hz / 1000 * info_p->min_scl_hightime_ns, NANO_TO_KILO);

    FI2C_INFO("i2c: mode %d, bus_clk %d, speed %d, period %d rise %d fall %d tlow %d "
              "thigh %d spk %d\n",
              speed_mode, bus_clk_hz, instance_p->config.speed_rate, period_cnt,
              rise_cnt, fall_cnt, min_t_low_cnt, min_t_high_cnt, spk_cnt);
    /*
     * Back-solve for hcnt and lcnt according to the following equations:
     * SCL_High_time = [(HCNT + IC_*_SPKLEN + 7) * icClk] + SCL_Fall_time
     * SCL_Low_time = [(LCNT + 1) * icClk] - SCL_Fall_time + SCL_Rise_time
     */
    hcnt = min_t_high_cnt - fall_cnt - 7 - spk_cnt;
    lcnt = min_t_low_cnt - rise_cnt + fall_cnt - 1;

    if (hcnt < 0 || lcnt < 0)
    {
        FI2C_ERROR("i2c: bad counts. hcnt = %d lcnt = %d\n", hcnt, lcnt);
        return FI2C_ERR_INVAL_PARM;
    }
    /*
     * Now add things back up to ensure the period is hit. If it is off,
     * split the difference and bias to lcnt for remainder
     */
    tot = hcnt + lcnt + 7 + spk_cnt + rise_cnt + 1;

    if (tot < period_cnt)
    {
        diff = (period_cnt - tot) / 2;
        hcnt += diff;
        lcnt += diff;
        tot = hcnt + lcnt + 7 + spk_cnt + rise_cnt + 1;
        lcnt += period_cnt - tot;
    }

    speed_cfg_p->scl_lcnt = lcnt;
    speed_cfg_p->scl_hcnt = hcnt;
    speed_cfg_p->sda_hold = DIV_ROUND_UP(bus_clk_hz / 1000 * 300, NANO_TO_KILO); /* Use internal default unless other value is specified */

    FI2C_INFO("i2c: hcnt = %d lcnt = %d sda hold = %d\n", speed_cfg_p->scl_hcnt,
              speed_cfg_p->scl_lcnt, speed_cfg_p->sda_hold);

    return ret;
}

/**
 * @name: FI2cSetTiming
 * @msg: I2C的上升沿下降沿配置
 * @return {*}
 * @param {FI2c} *instance_p ,i2c 控制器实例
 * @param {u32} speed_mode，i2c 速率模式
 * @note: 直接设置寄存器，不使用公式计算，其原理是根据i2c协议的时序要求，以及I2C模块接口频率计算公式
 * SCL_FREQ = I2C_CLK_FREQ / (SCL_L_CNT + SCL_H_CNT + 1 + 7 +I2C_SPKLEN)；其中SCL_FREQ为I2C通信速率，
 * SCL_L_CNT、SCL_H_CNT为SCL的上升沿和下降沿的计数值，I2C_SPKLEN为I2C时钟的脉冲长度，在100K及400K时取5,1M以上取2，I2C_CLK_FREQ为I2C总线时钟频率为50M。
 * 假定此时为1M传输速率,易得，1M = 50M / (SCL_L_CNT + SCL_H_CNT + 1 + 7 + 2)；则此时SCL_L_CNT + SCL_H_CNT为40即可，由于外围器件和I2C模块自身损耗，
 * 实际上若要严格的1M传输速率，SCL_L_CNT + SCL_H_CNT的应略小于40。此值可根据具体使用情况修改，但应保证SCL_L_CNT不小于6，SCL_H_CNT不小于8。
 *
 */
static FError FI2cSetTiming(FI2c *instance_p, FI2cSpeedCfg *speed_cfg_p)
{
    FASSERT(instance_p);
    uintptr addr = instance_p->config.base_addr;
    FError ret = FI2C_SUCCESS;
    u32 reg_val = FI2C_READ_REG32(addr, FI2C_CON_OFFSET);
    /* reset speed mode bits */
    reg_val = ((FI2C_READ_REG32(addr, FI2C_CON_OFFSET)) & (~FI2C_CON_SPEED_MASK));

    switch (speed_cfg_p->speed_mode)
    {
        case FI2C_STANDARD_SPEED:
            reg_val |= FI2C_CON_STD_SPEED;
            FI2C_WRITE_REG32(addr, FI2C_SS_SCL_HCNT_OFFSET, speed_cfg_p->scl_hcnt);
            FI2C_WRITE_REG32(addr, FI2C_SS_SCL_LCNT_OFFSET, speed_cfg_p->scl_lcnt);
            break;
        case FI2C_FAST_SPEED:
            reg_val |= FI2C_CON_FAST_SPEED;
            FI2C_WRITE_REG32(addr, FI2C_FS_SCL_HCNT_OFFSET, speed_cfg_p->scl_hcnt);
            FI2C_WRITE_REG32(addr, FI2C_FS_SCL_LCNT_OFFSET, speed_cfg_p->scl_lcnt);
            break;
        case FI2C_HIGH_SPEED:
            reg_val |= FI2C_CON_HIGH_SPEED;
            FI2C_WRITE_REG32(addr, FI2C_HS_SCL_HCNT_OFFSET, speed_cfg_p->scl_hcnt);
            FI2C_WRITE_REG32(addr, FI2C_HS_SCL_LCNT_OFFSET, speed_cfg_p->scl_lcnt);
            break;
        default:
            ret |= FI2C_ERR_INVAL_PARM;
            break;
    }
    FI2C_WRITE_REG32(addr, FI2C_SDA_HOLD_OFFSET, speed_cfg_p->sda_hold);
    FI2C_WRITE_REG32(addr, FI2C_CON_OFFSET, reg_val);
    return ret;
}

/**
 * @name: FI2cSetSpeed
 * @msg: 设置I2C控制器的速率
 * @return {*}
 * @param {FI2c} *instance_p, I2C控制器实例
 * @param {u32} speed_rate, I2C传输速率
 *  @param {u32} auto_calc, 直接设置寄存器或者利用公式计算
 */
FError FI2cSetSpeed(FI2c *instance_p, u32 speed_rate, boolean auto_calc)
{
    FASSERT(instance_p);
    FError ret = FI2C_SUCCESS;
    FI2cSpeedCfg speed_cfg;
    u32 spk_cnt = 0;
    uintptr addr = instance_p->config.base_addr;
    instance_p->config.auto_calc = auto_calc;
    memset(&speed_cfg, 0, sizeof(speed_cfg));

    if (FI2C_SPEED_HIGH_RATE <= speed_rate)
    {
        speed_cfg.speed_mode = FI2C_HIGH_SPEED;
        instance_p->config.speed_rate = FI2C_SPEED_HIGH_RATE;
        spk_cnt = FI2C_READ_REG32(addr, FI2C_HS_SPKLEN_OFFSET);
    }

    else if (FI2C_SPEED_FAST_RATE <= speed_rate)
    {
        speed_cfg.speed_mode = FI2C_FAST_SPEED;
        instance_p->config.speed_rate = FI2C_SPEED_FAST_RATE;
        spk_cnt = FI2C_READ_REG32(addr, FI2C_FS_SPKLEN_OFFSET);
    }
    else if (FI2C_SPEED_STANDARD_RATE <= speed_rate)
    {
        speed_cfg.speed_mode = FI2C_STANDARD_SPEED;
        instance_p->config.speed_rate = FI2C_SPEED_STANDARD_RATE;
        spk_cnt = FI2C_READ_REG32(addr, FI2C_FS_SPKLEN_OFFSET);
    }
    else
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /*  disenable setting for restore later */
    u32 enable_status = FI2cGetEnable(addr);
    if (FI2C_IC_ENABLE == enable_status)
    {
        ret = FI2cSetEnable(addr, FALSE);
    }
    if (auto_calc == TRUE)
    {
        FI2cCalcTiming(instance_p, spk_cnt, &speed_cfg);
    }
    else
    {
        speed_cfg = i2c_speed_reg_cfg[speed_cfg.speed_mode]; /*若不使用时序参数设置，则使用默认值*/
    }
    ret = FI2cSetTiming(instance_p, &speed_cfg);
    if (FI2C_SUCCESS != ret)
    {
        return ret;
    }

    /* enable i2c now speed set */
    if (FI2C_IC_ENABLE == enable_status)
    {
        ret = FI2cSetEnable(addr, TRUE);
    }

    if (FI2C_SUCCESS != ret)
    {
        return ret;
    }
    return ret;
}

/**
 * @name: FI2cSetTar
 * @msg: 设置与I2C主机通信的从机地址
 * @return {*}
 * @param {FI2c} *instance_p, I2C控制器实例
 * @param {u32} tar_addr, I2C从机地址
 */
FError FI2cSetTar(FI2c *instance_p, u32 tar_addr)
{
    FASSERT(instance_p);
    uintptr addr = instance_p->config.base_addr;
    u32 enable_status = FI2cGetEnable(addr);
    u32 ret = FI2C_SUCCESS;
    if (FI2C_IC_ENABLE == enable_status)
    {
        ret = FI2cSetEnable(addr, FALSE);
    }

    if (FI2C_SUCCESS == ret)
    {
        if ((instance_p->config.use_7bit_addr) == FALSE)
        {
            FI2C_WRITE_REG32(addr, FI2C_TAR_OFFSET, (tar_addr & FI2C_IC_TAR_MASK) | FI2C_TAR_ADR_10BIT);
        }
        else
        {
            FI2C_WRITE_REG32(addr, FI2C_TAR_OFFSET, (tar_addr & FI2C_IC_TAR_MASK) | FI2C_TAR_ADR_7BIT);
        }
    }
    if (FI2C_IC_ENABLE == enable_status)
    {
        ret = FI2cSetEnable(addr, TRUE);
    }
    return ret;
}

/**
 * @name: FI2cSetSar
 * @msg: 从机模式下，设置I2C地址
 * @return {*}
 * @param {FI2c} *instance_p, I2C控制器实例
 * @param {u32} sar_addr, 作为从机的地址
 */
FError FI2cSetSar(FI2c *instance_p, u32 sar_addr)
{
    FASSERT(instance_p);

    uintptr addr = instance_p->config.base_addr;
    u32 enable_status = FI2cGetEnable(addr);
    u32 ret = FI2C_SUCCESS;

    if (FI2C_IC_ENABLE == enable_status)
    {
        ret = FI2cSetEnable(addr, FALSE);
    }

    if (FI2C_SUCCESS == ret)
    {
        FI2C_WRITE_REG32(addr, FI2C_SAR_OFFSET, (sar_addr & FI2C_IC_SAR_MASK));
    }

    if (FI2C_IC_ENABLE == enable_status)
    {
        ret = FI2cSetEnable(addr, TRUE);
    }

    return ret;
}


/**
 * @name: FI2cSetAddress
 * @msg: 设置I2C控制器的从机地址
 * @return {*}
 * @param {FI2c} *instance_p, I2C控制器实例
 * @param {u32} work_mode, I2C工作模式
 *  @param {u32} slave_addr, I2C从机地址
 */

FError FI2cSetAddress(FI2c *instance_p, u32 work_mode, u32 slave_addr)
{
    FASSERT(instance_p);
    FError ret = FI2C_SUCCESS;
    uintptr base_addr = instance_p->config.base_addr;
    u32 reg_val = 0;
    ret = FI2cSetEnable(base_addr, FALSE); /* disable i2c ctrl */
    if (FI2C_MASTER == work_mode)
    {
        /* FI2C_CON_MASTER_ADR_7BIT : FI2C_CON_MASTER_ADR_10BIT 主机配置7位或10位地址方式，需要在 FI2C_TAR_OFFSET 第12位设置，FI2cSetTar()函数会设置 */
        reg_val |= FI2C_CON_SLAVE_DISABLE;
        reg_val |= FI2C_CON_MASTER_MODE;
        reg_val |= FI2C_CON_RESTART_EN;
    }
    else
    {
        reg_val |= (instance_p->config.use_7bit_addr) ? FI2C_CON_SLAVE_ADR_7BIT : FI2C_CON_SLAVE_ADR_10BIT;
        reg_val &= (~FI2C_CON_MASTER_MODE);
        reg_val |= FI2C_CON_SLAVE_MODE;
    }
    instance_p->config.work_mode = work_mode;
    instance_p->config.slave_addr = slave_addr;

    FI2C_WRITE_REG32(base_addr, FI2C_CON_OFFSET, reg_val);
    FI2C_WRITE_REG32(base_addr, FI2C_RX_TL_OFFSET, 0);
    FI2C_WRITE_REG32(base_addr, FI2C_TX_TL_OFFSET, 0);
    FI2C_SET_INTERRUPT_MASK(base_addr, 0); /* disable all intr */
    if (FI2C_SUCCESS == ret)
    {
        ret = FI2cSetEnable(base_addr, TRUE); /* enable i2c ctrl */
    }
    /* if init successed, and i2c is in slave mode, set slave address */
    if ((FI2C_SLAVE == work_mode))
    {
        ret = FI2cSetSar(instance_p, slave_addr);
    }

    return ret;
}
