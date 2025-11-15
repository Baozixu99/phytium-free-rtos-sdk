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
 * FilePath: fddma_g.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:24:47
 * Description:  This files is for ddma static configuration
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/5/13    init commit
 */

/***************************** Include Files *********************************/
#include "fparameters.h"

#include "fddma_hw.h"
#include "fddma.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
const FDdmaConfig fddma_cfg_tbl[FDDMA_INSTANCE_NUM] =
{
#if defined(FDDMA0_ID)
    [FDDMA0_ID] =
    {
        .id = FDDMA0_ID,
        .base_addr = FDDMA0_BASE_ADDR,
        .irq_num = FDDMA0_IRQ_NUM,
        .irq_prority = 0,
        .caps = FDDMA0_CAPACITY
    },
#endif
#if defined(FDDMA1_ID)
    [FDDMA1_ID] =
    {
        .id = FDDMA1_ID,
        .base_addr = FDDMA1_BASE_ADDR,
        .irq_num = FDDMA1_IRQ_NUM,
        .irq_prority = 0,
        .caps = FDDMA1_CAPACITY
    },
#endif
#if defined(FDDMA2_I2S_ID)
    [FDDMA2_I2S_ID] =
    {
        .id = FDDMA2_I2S_ID,
        .base_addr = FDDMA2_BASE_ADDR,
        .irq_num = FDDMA2_IRQ_NUM,
        .irq_prority = 1,
        .caps = FDDMA2_CAPACITY
    },
#endif
#if defined(FDDMA3_DP0_I2S_ID)
    [FDDMA3_DP0_I2S_ID] =
    {
        .id = FDDMA3_DP0_I2S_ID,
        .base_addr = FDDMA3_BASE_ADDR,
        .irq_num = FDDMA3_IRQ_NUM,
        .irq_prority = 1,
        .caps = FDDMA3_CAPACITY
    },
#endif
#if defined(FDDMA4_DP1_I2S_ID)
    [FDDMA4_DP1_I2S_ID] =
    {
        .id = FDDMA4_DP1_I2S_ID,
        .base_addr = FDDMA4_BASE_ADDR,
        .irq_num = FDDMA4_IRQ_NUM,
        .irq_prority = 1,
        .caps = FDDMA4_CAPACITY
    },
#endif
#if defined(FDDMA5_I2S_ID)
        [FDDMA5_I2S_ID] =
    {
        .id = FDDMA5_I2S_ID,
        .base_addr = FDDMA5_BASE_ADDR,
        .irq_num = FDDMA5_IRQ_NUM,
        .irq_prority = 1,
        .caps = FDDMA5_CAPACITY
    }
#endif
}; /* DDMA控制器默认配置 */
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/