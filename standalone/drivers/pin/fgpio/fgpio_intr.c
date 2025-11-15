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
 * FilePath: fgpio_intr.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:25:29
 * Description:  This files is for GPIO interrupt function implementation
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
#define FGPIO_DEBUG_TAG "FGPIO-INTR"
#define FGPIO_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_WARN(format, ...) FT_DEBUG_PRINT_W(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_INFO(format, ...) FT_DEBUG_PRINT_I(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
const FGpioConfig *FGpioLookupConfigByIrqNum(s32 irq_num);

/************************** Variable Definitions *****************************/
extern FGpioIntrMap fgpio_intr_map[FGPIO_CTRL_NUM];

/*****************************************************************************/
/**
 * @name: FGpioGetInterruptMask
 * @msg: 获取GPIO 引脚的中断屏蔽位
 * @return {*}
 * @param {FGpio} *pin, GPIO引脚实例
 * @param {u32} *mask, 返回的GPIO引脚中断屏蔽位
 * @param {u32} *enabled, 返回的GPIO引脚中断使能位
 */
void FGpioGetInterruptMask(FGpio *const pin, u32 *mask, u32 *enabled)
{
    FASSERT(pin);
    FASSERT_MSG(pin->is_ready == FT_COMPONENT_IS_READY,
                "gpio instance not is yet inited !!!");
    u32 reg_val;
    uintptr base_addr = pin->config.base_addr;

    if (NULL != mask)
    {
        reg_val = FGpioReadReg32(base_addr, FGPIO_INTMASK_OFFSET);
        *mask = reg_val & BIT(pin->config.pin);
    }

    if (NULL != enabled)
    {
        reg_val = FGpioReadReg32(base_addr, FGPIO_INTEN_OFFSET);
        *enabled = reg_val & BIT(pin->config.pin);
    }

    return;
}

/**
 * @name: FGpioSetInterruptMask
 * @msg: 设置GPIO引脚的中断屏蔽位
 * @return {*}
 * @param {FGpio} *pin, GPIO引脚实例
 * @param {boolean} enable, TRUE表示使能GPIO引脚中断，FALSE表示去使能GPIO引脚中断
 * @note 引脚必须为A组引脚，B组引脚不支持中断
 */
void FGpioSetInterruptMask(FGpio *const pin, boolean enable)
{
    FASSERT(pin);
    FASSERT_MSG(pin->is_ready == FT_COMPONENT_IS_READY,
                "gpio instance not is yet inited !!!");
    uintptr base_addr = pin->config.base_addr;
    u32 mask_bits = 0;
    u32 enable_bits = 0;

    mask_bits = FGpioReadReg32(base_addr, FGPIO_INTMASK_OFFSET);
    enable_bits = FGpioReadReg32(base_addr, FGPIO_INTEN_OFFSET);

    if (TRUE == enable)
    {
        mask_bits &= ~BIT(pin->config.pin);  /* not mask: 0 */
        enable_bits |= BIT(pin->config.pin); /* enable pin irq: 1 */
    }
    else
    {
        mask_bits |= BIT(pin->config.pin);    /* mask: 1  */
        enable_bits &= ~BIT(pin->config.pin); /* disable pin irq: 0 */
    }

    FGpioWriteReg32(base_addr, FGPIO_INTMASK_OFFSET, mask_bits);
    FGpioWriteReg32(base_addr, FGPIO_INTEN_OFFSET, enable_bits);

    return;
}

/**
 * @name: FGpioGetInterruptType
 * @msg: 获取GPIO引脚的中断类型和中断极性
 * @return {*}
 * @param {FGpio} *pin, GPIO引脚实例
 * @param {FGpioIrqType} *type, GPIO引脚中断触发类型
 */
void FGpioGetInterruptType(FGpio *const pin, FGpioIrqType *type)
{
    FASSERT(pin && type);
    FASSERT_MSG(pin->is_ready == FT_COMPONENT_IS_READY,
                "gpio instance not is yet inited !!!");
    uintptr base_addr = pin->config.base_addr;
    u32 pin_num = pin->config.pin;
    u32 levels;
    u32 polarity;

    if (pin->config.cap & FGPIO_CAP_IRQ_NONE)
    {
        FGPIO_ERROR("None interrupt support for GPIO %d-%d-%d !!!", pin->config.ctrl,
                    pin->config.port, pin->config.pin);
        return;
    }

    levels = FGpioReadReg32(base_addr, FGPIO_INTTYPE_LEVEL_OFFSET);
    polarity = FGpioReadReg32(base_addr, FGPIO_INT_POLARITY_OFFSET);

    if ((levels & BIT(pin_num)) && !(polarity & BIT(pin_num)))
    {
        *type = FGPIO_IRQ_TYPE_EDGE_FALLING;
    }
    else if ((levels & BIT(pin_num)) && (polarity & BIT(pin_num)))
    {
        *type = FGPIO_IRQ_TYPE_EDGE_RISING;
    }
    else if (!(levels & BIT(pin_num)) && !(polarity & BIT(pin_num)))
    {
        *type = FGPIO_IRQ_TYPE_LEVEL_LOW;
    }
    else if (!(levels & BIT(pin_num)) && (polarity & BIT(pin_num)))
    {
        *type = FGPIO_IRQ_TYPE_LEVEL_HIGH;
    }

    return;
}

/**
 * @name: FGpioSetInterruptType
 * @msg: 设置GPIO引脚的中断类型
 * @return {*}
 * @param {FGpio} *pin, GPIO引脚实例
 * @param {FGpioIrqType} type, GPIO引脚中断触发类型
 * @note index对应的引脚必须为A组引脚，B组引脚不支持中断
 */
void FGpioSetInterruptType(FGpio *const pin, const FGpioIrqType type)
{
    FASSERT(pin);
    FASSERT_MSG(pin->is_ready == FT_COMPONENT_IS_READY,
                "gpio instance not is yet inited !!!");
    uintptr base_addr = pin->config.base_addr;
    u32 level = 0;
    u32 polarity = 0;

    if (pin->config.cap & FGPIO_CAP_IRQ_NONE)
    {
        FGPIO_ERROR("None interrupt support for GPIO %d-%d-%d !!!", pin->config.ctrl,
                    pin->config.port, pin->config.pin);
        return;
    }

    level = FGpioReadReg32(base_addr, FGPIO_INTTYPE_LEVEL_OFFSET);
    polarity = FGpioReadReg32(base_addr, FGPIO_INT_POLARITY_OFFSET);

    switch (type)
    {
        case FGPIO_IRQ_TYPE_EDGE_FALLING:
            level |= BIT(pin->config.pin);     /* 1, 边沿敏感型 */
            polarity &= ~BIT(pin->config.pin); /* 0, 下降沿或低电平 */
            break;
        case FGPIO_IRQ_TYPE_EDGE_RISING:
            level |= BIT(pin->config.pin);    /* 1, 边沿敏感型 */
            polarity |= BIT(pin->config.pin); /* 1, 上升沿或高电平 */
            break;
        case FGPIO_IRQ_TYPE_LEVEL_LOW:
            level &= ~BIT(pin->config.pin);    /* 0, 电平敏感型 */
            polarity &= ~BIT(pin->config.pin); /* 0, 下降沿或低电平 */
            break;
        case FGPIO_IRQ_TYPE_LEVEL_HIGH:
            level &= ~BIT(pin->config.pin);   /* 0, 电平敏感型 */
            polarity |= BIT(pin->config.pin); /* 1, 上升沿或高电平 */
            break;
        default:
            break;
    }

    FGpioWriteReg32(base_addr, FGPIO_INTTYPE_LEVEL_OFFSET, level);
    FGpioWriteReg32(base_addr, FGPIO_INT_POLARITY_OFFSET, polarity);

    return;
}

/**
 * @name: FGpioCtrlInterruptHandler
 * @msg: GPIO中断处理函数(引脚通过控制器统一上报中断，共用中断号)
 * @return {*}
 * @param @param {FGpio} pin, GPIO引脚 
 */
static void FGpioSharedInterruptHandler(u32 ctrl_num)
{
    FGpioIntrMap *map = &fgpio_intr_map[ctrl_num];
    uintptr base_addr = map->base_addr;
    FASSERT(base_addr != 0U);
    u32 raw_status = FGpioReadReg32(base_addr, FGPIO_RAW_INTSTATUS_OFFSET);
    u32 enable_bits = FGpioReadReg32(base_addr, FGPIO_INTEN_OFFSET);
    u32 pin_num;

    if ((raw_status == 0) || (enable_bits == 0) || ((raw_status & enable_bits) == 0))
    {
        return;
    }

    for (pin_num = 0; (pin_num < (FGPIO_PIN_NUM * FGPIO_PORT_NUM)); pin_num++)
    {
        /* skip if interrupt not enabled for this pin */
        if ((raw_status & BIT(pin_num)) && (enable_bits & BIT(pin_num)))
        {
            /* ack interrupt */
            FGpioWriteReg32(base_addr, FGPIO_PORTA_EOI_OFFSET, BIT(pin_num));

            if (map->irq_cbs[pin_num] != NULL)
            {
                map->irq_cbs[pin_num](0U, map->irq_cb_params[pin_num]);
            }
        }
    }
}

/**
 * @name: FGpioPinInterruptHandler
 * @msg: GPIO中断处理函数(引脚单独上报中断)
 * @return {*}
 * @param @param {FGpio} pin, GPIO引脚 
 */
static void FGpioPinInterruptHandler(u32 ctrl_num, u32 pin_num)
{
    FGpioIntrMap *map = &fgpio_intr_map[ctrl_num];
    uintptr base_addr = map->base_addr;
    FASSERT(base_addr != 0U);
    u32 raw_status = FGpioReadReg32(base_addr, FGPIO_RAW_INTSTATUS_OFFSET);
    u32 enable_bits = FGpioReadReg32(base_addr, FGPIO_INTEN_OFFSET);

    if ((raw_status == 0) || ((enable_bits & BIT(pin_num)) == 0))
    {
        return;
    }

    /* ack interrupt */
    FGpioWriteReg32(base_addr, FGPIO_PORTA_EOI_OFFSET, BIT(pin_num));

    if (map->irq_cbs[pin_num] != NULL)
    {
        map->irq_cbs[pin_num](0U, map->irq_cb_params[pin_num]);
    }

    return;
}

/**
 * @name: FGpioInterruptHandler
 * @msg: GPIO 引脚中断处理函数
 * @return {*}
 * @param {s32} vector, 中断输入参数1
 * @param {void} *param, 中断输入参数2
 * @note 需要用户将此函数注册到Interrtup上，使能GPIO中断才能生效
 */
void FGpioInterruptHandler(s32 vector, void *param)
{
    FGPIO_DEBUG("vector = %d\r\n", vector);
    const FGpioConfig *pin_cfg = FGpioLookupConfigByIrqNum(vector);

    if (pin_cfg == NULL)
    {
        return;
    }
    else if (pin_cfg->cap & FGPIO_CAP_IRQ_BY_PIN)
    {
        FGpioPinInterruptHandler(pin_cfg->ctrl, pin_cfg->pin); /* 引脚独占中断处理 */
    }
    else if (pin_cfg->cap & FGPIO_CAP_IRQ_BY_CTRL)
    {
        FGpioSharedInterruptHandler(pin_cfg->ctrl); /* 控制器所属的引脚共用中断处理 */
    }
}

/**
 * @name: FGpioRegisterInterruptCB
 * @msg: 注册GPIO引脚中断回调函数(引脚通过控制器统一上报中断，共用中断号)
 * @return {*}
 * @param {FGpio} pin, GPIO引脚 
 * @param {FGpioInterruptCallback} cb, GPIO引脚中断回调函数
 * @param {void} *cb_param, GPIO引脚中断回调函数输入参数
 * @note 注册的回调函数在FGpioInterruptHandler中被调用
 */
void FGpioRegisterInterruptCB(FGpio *const pin, FGpioInterruptCallback cb, void *cb_param)
{
    FASSERT(pin && (pin->config.ctrl < FGPIO_CTRL_NUM));
    u32 ctrl_num = pin->config.ctrl;
    u32 pin_num = pin->config.port * FGPIO_PIN_NUM + pin->config.pin;
    FGpioIntrMap *map = &fgpio_intr_map[ctrl_num];

    map->irq_cbs[pin_num] = cb;
    map->irq_cb_params[pin_num] = cb_param;

    return;
}