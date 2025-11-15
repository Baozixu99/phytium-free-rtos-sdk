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
 * FilePath: fmio_g.c
 * Date: 2022-06-20 21:05:07
 * LastEditTime: 2022-06-20 21:05:07
 * Description:  This file is for mio static configuration
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming 2022/06/20    first commit
 */

/***************************** Include Files *********************************/
#include "fparameters.h"
#include "fmio_hw.h"
#include "fmio.h"

/************************** Constant Definitions *****************************/

const FMioConfig FMioConfigTbl[FMIO_NUM] =
{
#if defined(FMIO0_ID)
    [FMIO0_ID] =
    {
        .instance_id = FMIO0_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO0_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO0_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO0_ID)
    },
#endif
#if defined(FMIO1_ID)
    [FMIO1_ID] =
    {
        .instance_id = FMIO1_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO1_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO1_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO1_ID)
    },
#endif
#if defined(FMIO2_ID)
    [FMIO2_ID] =
    {
        .instance_id = FMIO2_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO2_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO2_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO2_ID)
    },
#endif
#if defined(FMIO3_ID)
    [FMIO3_ID] =
    {
        .instance_id = FMIO3_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO3_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO3_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO3_ID)
    },
#endif
#if defined(FMIO4_ID)
    [FMIO4_ID] =
    {
        .instance_id = FMIO4_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO4_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO4_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO4_ID)
    },
#endif
#if defined(FMIO5_ID)
    [FMIO5_ID] =
    {
        .instance_id = FMIO5_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO5_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO5_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO5_ID)
    },
#endif
#if defined(FMIO6_ID)
    [FMIO6_ID] =
    {
        .instance_id = FMIO6_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO6_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO6_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO6_ID)
    },
#endif
#if defined(FMIO7_ID)
    [FMIO7_ID] =
    {
        .instance_id = FMIO7_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO7_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO7_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO7_ID)
    },
#endif
#if defined(FMIO8_ID)
    [FMIO8_ID] =
    {
        .instance_id = FMIO8_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO8_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO8_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO8_ID)
    },
#endif
#if defined(FMIO9_ID)
    [FMIO9_ID] =
    {
        .instance_id = FMIO9_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO9_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO9_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO9_ID)
    },
#endif
#if defined(FMIO10_ID)
    [FMIO10_ID] =
    {
        .instance_id = FMIO10_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO10_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO10_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO10_ID)
    },
#endif
#if defined(FMIO11_ID)
    [FMIO11_ID] =
    {
        .instance_id = FMIO11_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO11_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO11_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO11_ID)
    },
#endif
#if defined(FMIO12_ID)
    [FMIO12_ID] =
    {
        .instance_id = FMIO12_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO12_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO12_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO12_ID)
    },
#endif
#if defined(FMIO13_ID)
    [FMIO13_ID] =
    {
        .instance_id = FMIO13_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO13_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO13_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO13_ID)
    },
#endif
#if defined(FMIO14_ID)
    [FMIO14_ID] =
    {
        .instance_id = FMIO14_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO14_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO14_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO14_ID)
    },
#endif
#if defined(FMIO15_ID)
    [FMIO15_ID] =
    {
        .instance_id = FMIO15_ID,
        .func_base_addr = FMIO_BASE_ADDR(FMIO15_ID),
        .irq_num = FMIO_IRQ_NUM(FMIO15_ID),
        .mio_base_addr = FMIO_BASE_SET_ADDR(FMIO15_ID)
    }
#endif
};
