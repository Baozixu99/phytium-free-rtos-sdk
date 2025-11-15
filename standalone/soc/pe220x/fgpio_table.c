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
    FGPIO_PIN_CONFIG(FGPIO2_BASE_ADDR, FGPIO_CTRL_2, FGPIO_PORT_A, (pin), (irq), FGPIO_CAP_IRQ_BY_PIN)
#define FGPIO_PIN_CONFIG_3(pin, irq) \
    FGPIO_PIN_CONFIG(FGPIO3_BASE_ADDR, FGPIO_CTRL_3, FGPIO_PORT_A, (pin), (irq), FGPIO_CAP_IRQ_BY_CTRL)
#define FGPIO_PIN_CONFIG_4(pin, irq) \
    FGPIO_PIN_CONFIG(FGPIO4_BASE_ADDR, FGPIO_CTRL_4, FGPIO_PORT_A, (pin), (irq), FGPIO_CAP_IRQ_BY_CTRL)
#define FGPIO_PIN_CONFIG_5(pin, irq) \
    FGPIO_PIN_CONFIG(FGPIO5_BASE_ADDR, FGPIO_CTRL_5, FGPIO_PORT_A, (pin), (irq), FGPIO_CAP_IRQ_BY_CTRL)

const FGpioConfig fgpio_cfg_tbl[FGPIO_NUM] = {
    /* GPIO 0, IRQ 140 ~ 155 */
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_0, 140U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_1, 141U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_2, 142U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_3, 143U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_4, 144U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_5, 145U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_6, 146U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_7, 147U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_8, 148U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_9, 149U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_10, 150U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_11, 151U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_12, 152U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_13, 153U),
    FGPIO_PIN_CONFIG_0(FGPIO_PIN_14, 154U), FGPIO_PIN_CONFIG_0(FGPIO_PIN_15, 155U),

    /* GPIO 1, IRQ 156 ~ 171 */
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_0, 156U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_1, 157U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_2, 158U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_3, 159U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_4, 160U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_5, 161U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_6, 162U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_7, 163U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_8, 164U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_9, 165U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_10, 166U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_11, 167U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_12, 168U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_13, 169U),
    FGPIO_PIN_CONFIG_1(FGPIO_PIN_14, 170U), FGPIO_PIN_CONFIG_1(FGPIO_PIN_15, 171U),

    /* GPIO 2, IRQ 172 ~ 187 */
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_0, 172U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_1, 173U),
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_2, 174U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_3, 175U),
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_4, 176U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_5, 177U),
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_6, 178U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_7, 179U),
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_8, 180U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_9, 181U),
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_10, 182U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_11, 183U),
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_12, 184U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_13, 185U),
    FGPIO_PIN_CONFIG_2(FGPIO_PIN_14, 186U), FGPIO_PIN_CONFIG_2(FGPIO_PIN_15, 187U),

    /* GPIO 3, IRQ 188 */
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_0, 188U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_1, 188U),
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_2, 188U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_3, 188U),
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_4, 188U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_5, 188U),
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_6, 188U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_7, 188U),
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_8, 188U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_9, 188U),
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_10, 188U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_11, 188U),
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_12, 188U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_13, 188U),
    FGPIO_PIN_CONFIG_3(FGPIO_PIN_14, 188U), FGPIO_PIN_CONFIG_3(FGPIO_PIN_15, 188U),

    /* GPIO 4, IRQ 189 */
    FGPIO_PIN_CONFIG_4(FGPIO_PIN_0, 189U), FGPIO_PIN_CONFIG_4(FGPIO_PIN_1, 189U),
    FGPIO_PIN_CONFIG_4(FGPIO_PIN_2, 189U), FGPIO_PIN_CONFIG_4(FGPIO_PIN_3, 189U),
    FGPIO_PIN_CONFIG_4(FGPIO_PIN_4, 189U), FGPIO_PIN_CONFIG_4(FGPIO_PIN_5, 189U),
    FGPIO_PIN_CONFIG_4(FGPIO_PIN_6, 189U), FGPIO_PIN_CONFIG_4(FGPIO_PIN_7, 189U),
    FGPIO_PIN_CONFIG_4(FGPIO_PIN_8, 189U), FGPIO_PIN_CONFIG_4(FGPIO_PIN_9, 189U),
    FGPIO_PIN_CONFIG_4(FGPIO_PIN_10, 189U), FGPIO_PIN_CONFIG_4(FGPIO_PIN_11, 189U),
    FGPIO_PIN_CONFIG_4(FGPIO_PIN_12, 189U), FGPIO_PIN_CONFIG_4(FGPIO_PIN_13, 189U),
    FGPIO_PIN_CONFIG_4(FGPIO_PIN_14, 189U), FGPIO_PIN_CONFIG_4(FGPIO_PIN_15, 189U),

    /* GPIO 5, IRQ 190 */
    FGPIO_PIN_CONFIG_5(FGPIO_PIN_0, 190U), FGPIO_PIN_CONFIG_5(FGPIO_PIN_1, 190U),
    FGPIO_PIN_CONFIG_5(FGPIO_PIN_2, 190U), FGPIO_PIN_CONFIG_5(FGPIO_PIN_3, 190U),
    FGPIO_PIN_CONFIG_5(FGPIO_PIN_4, 190U), FGPIO_PIN_CONFIG_5(FGPIO_PIN_5, 190U),
    FGPIO_PIN_CONFIG_5(FGPIO_PIN_6, 190U), FGPIO_PIN_CONFIG_5(FGPIO_PIN_7, 190U),
    FGPIO_PIN_CONFIG_5(FGPIO_PIN_8, 190U), FGPIO_PIN_CONFIG_5(FGPIO_PIN_9, 190U),
    FGPIO_PIN_CONFIG_5(FGPIO_PIN_10, 190U), FGPIO_PIN_CONFIG_5(FGPIO_PIN_11, 190U),
    FGPIO_PIN_CONFIG_5(FGPIO_PIN_12, 190U), FGPIO_PIN_CONFIG_5(FGPIO_PIN_13, 190U),
    FGPIO_PIN_CONFIG_5(FGPIO_PIN_14, 190U), FGPIO_PIN_CONFIG_5(FGPIO_PIN_15, 190U)};

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
    FGPIO_INTR_MAP_CONFIG(FGPIO3_BASE_ADDR),

    /* GPIO 4 IRQ Map */
    FGPIO_INTR_MAP_CONFIG(FGPIO4_BASE_ADDR),

    /* GPIO 5 IRQ Map */
    FGPIO_INTR_MAP_CONFIG(FGPIO5_BASE_ADDR)};

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