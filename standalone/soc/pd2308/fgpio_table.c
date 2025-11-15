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

#define FGPIO_PIN_CONFIG(_base, _ctrl, _port, _pin, _irq, _cap)                 \
    {                                                                           \
        .id = FGPIO_ID(_ctrl, _pin), .ctrl = _ctrl, .port = _port, .pin = _pin, \
        .base_addr = _base, .irq_num = _irq, .cap = _cap                        \
    }

#define FGPIO_PIN_CONFIG_0(pin, irq) \
    FGPIO_PIN_CONFIG(FGPIO0_BASE_ADDR, FGPIO_CTRL_0, FGPIO_PORT_A, (pin), (irq), FGPIO_CAP_IRQ_BY_PIN)
#define FGPIO_PIN_CONFIG_1(pin, irq) \
    FGPIO_PIN_CONFIG(FGPIO1_BASE_ADDR, FGPIO_CTRL_1, FGPIO_PORT_A, (pin), (irq), FGPIO_CAP_IRQ_BY_PIN)
#define FGPIO_PIN_CONFIG_2(pin, irq) \
    FGPIO_PIN_CONFIG(FGPIO2_BASE_ADDR, FGPIO_CTRL_2, FGPIO_PORT_A, (pin), (irq), FGPIO_CAP_IRQ_BY_CTRL)
#define FGPIO_PIN_CONFIG_3(pin, irq) \
    FGPIO_PIN_CONFIG(FGPIO3_BASE_ADDR, FGPIO_CTRL_3, FGPIO_PORT_A, (pin), (irq), FGPIO_CAP_IRQ_BY_CTRL)

const FGpioConfig fgpio_cfg_tbl[FGPIO_NUM] = {
    /* GPIO-0, IRQ 128 ~ 143 */
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_0, 128U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_1, 129U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_2, 130U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_3, 131U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_4, 132U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_5, 133U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_6, 134U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_7, 135U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_8, 136U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_9, 137U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_10, 138U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_11, 139U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_12, 140U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_13, 141U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_14, 142U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_15, 143U),

    /* GPIO-1, IRQ 144 ~ 159 */
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_0, 144U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_1, 145U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_2, 146U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_3, 147U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_4, 148U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_5, 149U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_6, 150U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_7, 151U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_8, 152U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_9, 153U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_10, 154U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_11, 155U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_12, 156U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_13, 157U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_14, 158U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_15, 159U),

    /* GPIO-2, IRQ 160 */
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_0, 160U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_1, 160U),
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_2, 160U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_3, 160U),
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_4, 160U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_5, 160U),
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_6, 160U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_7, 160U),
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_8, 160U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_9, 160U),
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_10, 160U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_11, 160U),
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_12, 160U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_13, 160U),
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_14, 160U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_15, 160U),

    /* GPIO-3 IRQ 161 */
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_0, 161U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_1, 161U),
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_2, 161U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_3, 161U),
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_4, 161U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_5, 161U),
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_6, 161U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_7, 161U),
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_8, 161U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_9, 161U),
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_10, 161U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_11, 161U),
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_12, 161U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_13, 161U),
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_14, 161U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_15, 161U)};


#define FGPIO_INTR_MAP_CONFIG(_base)                                    \
    {                                                                   \
        .base_addr = _base, .irq_cbs = {NULL}, .irq_cb_params = {NULL}, \
    }

FGpioIntrMap fgpio_intr_map[FGPIO_CTRL_NUM] = {
    /* GPIO 0 IRQ Map */
    FGPIO_INTR_MAP_CONFIG(FGPIO0_BASE_ADDR),

    /* GPIO 1 IRQ Map */
    FGPIO_INTR_MAP_CONFIG(FGPIO1_BASE_ADDR),

    /* GPIO 2 IRQ Map */
    FGPIO_INTR_MAP_CONFIG(FGPIO2_BASE_ADDR),

    /* GPIO 3 IRQ Map */
    FGPIO_INTR_MAP_CONFIG(FGPIO3_BASE_ADDR)};

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