/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * 1.0   wangzongqiang  2024/8/8       init commit
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

const FGpioConfig fgpio_cfg_tbl[FGPIO_NUM] = {
    /* GPIO-0, IRQ 144 ~ 159 */
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_0, 144U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_1, 145U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_2, 146U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_3, 147U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_4, 148U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_5, 149U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_6, 150U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_7, 151U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_8, 152U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_9, 153U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_10, 154U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_11, 155U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_12, 156U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_13, 157U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_14, 158U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_15, 159U),

    /* GPIO-1, IRQ 160 ~ 175 */
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_0, 160U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_1, 161U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_2, 162U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_3, 163U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_4, 164U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_5, 165U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_6, 166U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_7, 167U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_8, 168U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_9, 169U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_10, 170U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_11, 171U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_12, 172U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_13, 173U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_14, 174U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_15, 175U)};


#define FGPIO_INTR_MAP_CONFIG(_base)                                    \
    {                                                                   \
        .base_addr = _base, .irq_cbs = {NULL}, .irq_cb_params = {NULL}, \
    }

FGpioIntrMap fgpio_intr_map[FGPIO_CTRL_NUM] = {
    /* GPIO 0 IRQ Map */
    FGPIO_INTR_MAP_CONFIG(FGPIO0_BASE_ADDR),

    /* GPIO 1 IRQ Map */
    FGPIO_INTR_MAP_CONFIG(FGPIO1_BASE_ADDR),

};

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