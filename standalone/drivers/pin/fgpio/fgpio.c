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
 * FilePath: fgpio.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:25:29
 * Description:  This files is for GPIO user API implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/3/1     init commit
 * 2.0   zhugengyu  2022/7/1     support e2000
 * 3.0   zhugengyu  2024/5/7     modify interface to use gpio by pin
 */


/***************************** Include Files *********************************/
#include "fdrivers_port.h"
#include "fparameters.h"

#include "fgpio_hw.h"
#include "fgpio.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FGPIO_DEBUG_TAG "FGPIO"
#define FGPIO_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_WARN(format, ...) FT_DEBUG_PRINT_W(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_INFO(format, ...) FT_DEBUG_PRINT_I(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @name: FGpioCfgInitialize
 * @msg: 初始化GPIO引脚实例
 * @return {FError} FGPIO_SUCCESS 表示初始化成功
 * @param {FGpio} *pin, GPIO引脚实例
 * @param {FGpioConfig} *config, GPIO引脚配置
 */
FError FGpioCfgInitialize(FGpio *const pin, const FGpioConfig *const config)
{
    FASSERT(pin && config);

    if (0 == config->base_addr)
    {
        FGPIO_ERROR("Invalid base address !!!");
        return FGPIO_ERR_INVALID_PARA;
    }

    if (config != &pin->config)
    {
        pin->config = *config;
    }

    pin->is_ready = FT_COMPONENT_IS_READY;

    /* deregister interrupt callback */
    FGpioRegisterInterruptCB(pin, NULL, NULL);

    /* disable interrupt*/
    FGpioSetInterruptMask(pin, FALSE);

#ifdef FGPIO_PORT_B
    FGPIO_INFO("GPIO %d-%d-%d inited", pin->config.ctrl, pin->config.port, pin->config.pin);
#else
    FGPIO_INFO("GPIO %d-%d inited", pin->config.ctrl, pin->config.pin);
#endif
    return FGPIO_SUCCESS;
}

/**
 * @name: FGpioDeInitialize
 * @msg: 去初始化GPIO引脚实例
 * @return {*}
 * @param {FGpio} *pin, GPIO引脚实例
 */
void FGpioDeInitialize(FGpio *const pin)
{
    FASSERT(pin);

    /* 关闭引脚中断 */
    FGpioSetInterruptMask(pin, FALSE);

    pin->is_ready = 0;
    return;
}

/**
 * @name: FGpioReadRegDir
 * @msg: 从寄存器读取GPIO组的输入输出方向
 * @return {u32} GPIO组的输入输出方向, bit[8:0]有效
 * @param {FGpio} *pin, GPIO引脚实例
 * @param {FGpioPortIndex} port, GPIO组, A/B
 */
static inline u32 FGpioReadRegDir(FGpio *const pin)
{
    u32 reg_val = 0;
    uintptr base_addr = pin->config.base_addr;

#ifdef FGPIO_PORT_A
    if (pin->config.port == FGPIO_PORT_A)
    {
        reg_val = FGpioReadReg32(base_addr, FGPIO_SWPORTA_DDR_OFFSET);
    }
#endif
#ifdef FGPIO_PORT_B
    if (pin->config.port == FGPIO_PORT_B)
    {
        reg_val = FGpioReadReg32(base_addr, FGPIO_SWPORTB_DDR_OFFSET);
    }
#endif

    return reg_val;
}

/**
 * @name: FGpioWriteRegDir
 * @msg: 向寄存器写入GPIO组的输入输出方向
 * @return {*}
 * @param {FGpio} *pin, GPIO引脚实例
 * @param {FGpioPortIndex} port, GPIO组, A/B
 * @param {u32} reg_val, GPIO组的输入输出方向, bit[8:0]有效
 */
static inline void FGpioWriteRegDir(FGpio *const pin, const u32 reg_val)
{
    uintptr base_addr = pin->config.base_addr;

#ifdef FGPIO_PORT_A
    if (pin->config.port == FGPIO_PORT_A)
    {
        FGpioWriteReg32(base_addr, FGPIO_SWPORTA_DDR_OFFSET, reg_val);
    }
#endif
#ifdef FGPIO_PORT_B
    if (pin->config.port == FGPIO_PORT_B)
    {
        FGpioWriteReg32(base_addr, FGPIO_SWPORTB_DDR_OFFSET, reg_val);
    }
#endif

    return;
}

/**
 * @name: FGpioSetDirection
 * @msg: 设置GPIO引脚的输入输出方向
 * @return {*}
 * @param {FGpio} *pin, GPIO引脚实例
 * @param {FGpioDirection} dir, 待设置的GPIO的方向
 * @note 初始化 GPIO 实例后使用此函数
 */
void FGpioSetDirection(FGpio *const pin, FGpioDirection dir)
{
    FASSERT(pin);
    FASSERT_MSG(pin->is_ready == FT_COMPONENT_IS_READY,
                "gpio instance is not yet inited !!!");
    u32 reg_val;

    reg_val = FGpioReadRegDir(pin);

    if (FGPIO_DIR_INPUT == dir)
    {
        reg_val &= ~BIT(pin->config.pin); /* 0-Input */
    }
    else if (FGPIO_DIR_OUTPUT == dir)
    {
        reg_val |= BIT(pin->config.pin); /* 1-Output */
    }
    else
    {
        FASSERT(0);
    }

    FGpioWriteRegDir(pin, reg_val);
    return;
}

/**
 * @name: FGpioGetDirection
 * @msg: 获取GPIO引脚的输入输出方向
 * @return {FGpioDirection} GPIO引脚方向
 * @param {FGpio} *pin, GPIO引脚实例
 * @note 初始化 GPIO 实例后使用此函数
 */
FGpioDirection FGpioGetDirection(FGpio *const pin)
{
    FASSERT(pin);
    FASSERT_MSG(pin->is_ready == FT_COMPONENT_IS_READY,
                "gpio instance is not yet inited !!!");
    u32 reg_val = FGpioReadRegDir(pin);
    return (BIT(pin->config.pin) & reg_val) ? FGPIO_DIR_OUTPUT : FGPIO_DIR_INPUT;
}

/**
 * @name: FGpioReadRegVal
 * @msg: 获取GPIO组的输出寄存器值
 * @return {u32} 输出寄存器值 bit[8:0]有效
 * @param {FGpio} pin, GPIO引脚实例
 */
static inline u32 FGpioReadRegVal(FGpio *const pin)
{
    u32 reg_val = 0;
    uintptr base_addr = pin->config.base_addr;

#ifdef FGPIO_PORT_A
    if (pin->config.port == FGPIO_PORT_A)
    {
        reg_val = FGpioReadReg32(base_addr, FGPIO_SWPORTA_DR_OFFSET);
    }
#endif
#ifdef FGPIO_PORT_B
    if (pin->config.port == FGPIO_PORT_B)
    {
        reg_val = FGpioReadReg32(base_addr, FGPIO_SWPORTB_DR_OFFSET);
    }
#endif

    return reg_val;
}

/**
 * @name: FGpioWriteRegVal
 * @msg: 设置GPIO组的输出寄存器值
 * @return {*}
 * @param {uintptr} base_addr, GPIO控制器基地址
 * @param {FGpio} pin, GPIO引脚实例
 * @param {u32} reg_val, 输出寄存器值有效
 */
static inline void FGpioWriteRegVal(FGpio *const pin, const u32 reg_val)
{
    uintptr base_addr = pin->config.base_addr;

#ifdef FGPIO_PORT_A
    if (pin->config.port == FGPIO_PORT_A)
    {
        FGpioWriteReg32(base_addr, FGPIO_SWPORTA_DR_OFFSET, reg_val);
    }
#endif
#ifdef FGPIO_PORT_B
    if (pin->config.port == FGPIO_PORT_B)
    {
        FGpioWriteReg32(base_addr, FGPIO_SWPORTB_DR_OFFSET, reg_val);
    }
#endif

    return;
}

/**
 * @name: FGpioReadRegInput
 * @msg: 获取GPIO引脚输入寄存器值
 * @return {*}
 * @param {FGpio} pin, GPIO引脚实例
 * @param {u32} reg_val, 输出寄存器值有效
 */
static inline u32 FGpioReadRegInput(FGpio *const pin)
{
    u32 reg_val = 0;
    uintptr base_addr = pin->config.base_addr;

#ifdef FGPIO_PORT_A
    if (pin->config.port == FGPIO_PORT_A)
    {
        reg_val = FGpioReadReg32(base_addr, FGPIO_EXT_PORTA_OFFSET);
    }
#endif
#ifdef FGPIO_PORT_B
    if (pin->config.port == FGPIO_PORT_B)
    {
        reg_val = FGpioReadReg32(base_addr, FGPIO_EXT_PORTB_OFFSET);
    }
#endif

    return reg_val;
}

/**
 * @name: FGpioSetOutputValue
 * @msg: 设置GPIO引脚的输出值
 * @return {FError} FGPIO_SUCCESS 表示设置成功
 * @param {FGpio} *pin, GPIO引脚实例
 * @param {FGpioVal} output, GPIO引脚的输出值
 * @note 初始化 GPIO 实例后使用此函数，先设置 GPIO 引脚为输出后调用此函数
 */
FError FGpioSetOutputValue(FGpio *const pin, const FGpioVal output)
{
    FASSERT(pin);
    FASSERT_MSG(pin->is_ready == FT_COMPONENT_IS_READY,
                "gpio instance is not yet inited !!!");

    u32 reg_val;

    if (FGPIO_DIR_OUTPUT != FGpioGetDirection(pin))
    {
        FGPIO_ERROR("Need to set GPIO direction as OUTPUT first !!!");
        return FGPIO_ERR_INVALID_STATE;
    }

    reg_val = FGpioReadRegVal(pin);
    if (FGPIO_PIN_LOW == output)
    {
        reg_val &= ~BIT(pin->config.pin);
    }
    else if (FGPIO_PIN_HIGH == output)
    {
        reg_val |= BIT(pin->config.pin);
    }
    else
    {
        FASSERT(0);
    }

    FGpioWriteRegVal(pin, reg_val);
    return FGPIO_SUCCESS;
}

/**
 * @name: FGpioGetInputValue
 * @msg: 获取GPIO引脚的输入值
 * @return {FGpioVal} 获取的输入值，高电平/低电平
 * @param {FGpio} *instance, GPIO引脚实例
 * @note 初始化 GPIO 实例后使用此函数，先设置 GPIO 引脚为输入后调用此函数
 */
FGpioVal FGpioGetInputValue(FGpio *const pin)
{
    FASSERT(pin);
    FASSERT_MSG(pin->is_ready == FT_COMPONENT_IS_READY,
                "gpio instance not is yet inited !!!");

    u32 reg_val;

    if (FGPIO_DIR_INPUT != FGpioGetDirection(pin))
    {
        FGPIO_ERROR("Need to set GPIO direction as INPUT first !!!");
        return FGPIO_PIN_LOW;
    }

    reg_val = FGpioReadRegInput(pin);

    FGPIO_INFO("Input val: 0x%x.", reg_val);
    return (BIT(pin->config.pin) & reg_val) ? FGPIO_PIN_HIGH : FGPIO_PIN_LOW;
}