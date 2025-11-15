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
 * FilePath: fpl011_g.c
 * Date: 2021-11-02 14:53:42
 * LastEditTime: 2022-02-18 09:05:41
 * Description:  This file is for uart static configuration
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  huanghe 2021/11/2  first commit
 * 1.1  liushengming 2022/02/18 add configs
 */

/***************************** Include Files *********************************/

#include "fpl011.h"
#include "fparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

const FPl011Config FPl011ConfigTable[FUART_NUM] =
{
#if defined(FUART0_ID)
    {
        .instance_id = FUART0_ID,
        .base_address = FUART0_BASE_ADDR,
        .ref_clock_hz = FUART0_CLK_FREQ_HZ,
        .irq_num = FUART0_IRQ_NUM,
        .baudrate = UART_DEFAULT_BAUD_RATE,
        .init_flg = 0
    },
#endif
#if defined(FUART1_ID)
    {
        .instance_id = FUART1_ID,
        .base_address = FUART1_BASE_ADDR,
        .ref_clock_hz = FUART1_CLK_FREQ_HZ,
        .irq_num = FUART1_IRQ_NUM,
        .baudrate = UART_DEFAULT_BAUD_RATE,
        .init_flg = 0
    },
#endif
#if defined(FUART2_ID)
    {
        .instance_id = FUART2_ID,
        .base_address = FUART2_BASE_ADDR,
        .ref_clock_hz = FUART2_CLK_FREQ_HZ,
        .irq_num = FUART2_IRQ_NUM,
        .baudrate = UART_DEFAULT_BAUD_RATE,
        .init_flg = 0
    },
#endif

#if defined(FUART3_ID)
    {
        .instance_id = FUART3_ID,
        .base_address = FUART3_BASE_ADDR,
        .ref_clock_hz = FUART3_CLK_FREQ_HZ,
        .irq_num = FUART3_IRQ_NUM,
        .baudrate = UART_DEFAULT_BAUD_RATE,
        .init_flg = 0
    }
#endif 

};
