/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fgpio_table.c
 * Date: 2023-11-6 10:33:28
 * LastEditTime: 2023-11-6 10:33:28
 * Description:  This file is for GPIO pin definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2024/5/9       init commit
 */

#include "fparameters.h"
#include "fgpio.h"

#define FGPIO_PIN_CONFIG(_base, _ctrl, _port, _pin, _irq, _cap)                        \
    {                                                                                  \
        .id = FGPIO_ID(_ctrl, _port, _pin), .ctrl = _ctrl, .port = _port, .pin = _pin, \
        .base_addr = _base, .irq_num = _irq, .cap = _cap                               \
    }

#define FGPIO_PIN_CONFIG_0_A(pin, irq) \
    FGPIO_PIN_CONFIG(FGPIO0_BASE_ADDR, FGPIO_CTRL_0, FGPIO_PORT_A, (pin), (irq), FGPIO_CAP_IRQ_BY_CTRL)
#define FGPIO_PIN_CONFIG_0_B(pin, irq) \
    FGPIO_PIN_CONFIG(FGPIO0_BASE_ADDR, FGPIO_CTRL_0, FGPIO_PORT_B, (pin), (irq), FGPIO_CAP_IRQ_NONE)
#define FGPIO_PIN_CONFIG_1_A(pin, irq) \
    FGPIO_PIN_CONFIG(FGPIO1_BASE_ADDR, FGPIO_CTRL_1, FGPIO_PORT_A, (pin), (irq), FGPIO_CAP_IRQ_BY_CTRL)
#define FGPIO_PIN_CONFIG_1_B(pin, irq) \
    FGPIO_PIN_CONFIG(FGPIO1_BASE_ADDR, FGPIO_CTRL_1, FGPIO_PORT_B, (pin), (irq), FGPIO_CAP_IRQ_NONE)

const FGpioConfig fgpio_cfg_tbl[FGPIO_NUM] = {
    /* GPIO 0-A, IRQ 42 */
    FGPIO_PIN_CONFIG_0_A(FGPIO_PIN_0, 42U), FGPIO_PIN_CONFIG_0_A(FGPIO_PIN_1, 42U),
    FGPIO_PIN_CONFIG_0_A(FGPIO_PIN_2, 42U), FGPIO_PIN_CONFIG_0_A(FGPIO_PIN_3, 42U),
    FGPIO_PIN_CONFIG_0_A(FGPIO_PIN_4, 42U), FGPIO_PIN_CONFIG_0_A(FGPIO_PIN_5, 42U),
    FGPIO_PIN_CONFIG_0_A(FGPIO_PIN_6, 42U), FGPIO_PIN_CONFIG_0_A(FGPIO_PIN_7, 42U),

    /* GPIO 0-B, None IRQ */
    FGPIO_PIN_CONFIG_0_B(FGPIO_PIN_0, 0U), FGPIO_PIN_CONFIG_0_B(FGPIO_PIN_1, 0U),
    FGPIO_PIN_CONFIG_0_B(FGPIO_PIN_2, 0U), FGPIO_PIN_CONFIG_0_B(FGPIO_PIN_3, 0U),
    FGPIO_PIN_CONFIG_0_B(FGPIO_PIN_4, 0U), FGPIO_PIN_CONFIG_0_B(FGPIO_PIN_5, 0U),
    FGPIO_PIN_CONFIG_0_B(FGPIO_PIN_6, 0U), FGPIO_PIN_CONFIG_0_B(FGPIO_PIN_7, 0U),

    /* GPIO 1-A IRQ 43 */
    FGPIO_PIN_CONFIG_1_A(FGPIO_PIN_0, 43U), FGPIO_PIN_CONFIG_1_A(FGPIO_PIN_1, 43U),
    FGPIO_PIN_CONFIG_1_A(FGPIO_PIN_2, 43U), FGPIO_PIN_CONFIG_1_A(FGPIO_PIN_3, 43U),
    FGPIO_PIN_CONFIG_1_A(FGPIO_PIN_4, 43U), FGPIO_PIN_CONFIG_1_A(FGPIO_PIN_5, 43U),
    FGPIO_PIN_CONFIG_1_A(FGPIO_PIN_6, 43U), FGPIO_PIN_CONFIG_1_A(FGPIO_PIN_7, 43U),

    /* GPIO 1-B None IRQ */
    FGPIO_PIN_CONFIG_1_B(FGPIO_PIN_0, 0U), FGPIO_PIN_CONFIG_1_B(FGPIO_PIN_1, 0U),
    FGPIO_PIN_CONFIG_1_B(FGPIO_PIN_2, 0U), FGPIO_PIN_CONFIG_1_B(FGPIO_PIN_3, 0U),
    FGPIO_PIN_CONFIG_1_B(FGPIO_PIN_4, 0U), FGPIO_PIN_CONFIG_1_B(FGPIO_PIN_5, 0U),
    FGPIO_PIN_CONFIG_1_B(FGPIO_PIN_6, 0U), FGPIO_PIN_CONFIG_1_B(FGPIO_PIN_7, 0U)};

#define FGPIO_INTR_MAP_CONFIG(_base)                                    \
    {                                                                   \
        .base_addr = _base, .irq_cbs = {NULL}, .irq_cb_params = {NULL}, \
    }

FGpioIntrMap fgpio_intr_map[FGPIO_CTRL_NUM] = {
    /* GPIO 0 IRQ Map */
    FGPIO_INTR_MAP_CONFIG(FGPIO0_BASE_ADDR),

    /* GPIO 1 IRQ Map */
    FGPIO_INTR_MAP_CONFIG(FGPIO1_BASE_ADDR)};

const FGpioConfig *FGpioLookupConfigByIrqNum(s32 irq_num)
{
    u32 index;
    const FGpioConfig *ptr = NULL;

    for (index = 0; index < FGPIO_NUM; index++)
    {
        /* 如果引脚单独上报中断，返回对应引脚的配置
           如果引脚通过控制器统一上报中断，共用中断号，返回使用该中断号的第一个引脚的配置
           如果引脚不支持中断，返回 NULL  */
        if (fgpio_cfg_tbl[index].irq_num == irq_num)
        {
            ptr = &fgpio_cfg_tbl[index];
            break;
        }
    }

    return ptr;
}