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
 * FilePath: fi2s_g.c
 * Created Date: 2023-05-09 09:59:03
 * Last Modified: 2024-04-19 10:12:52
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/05/09  Modify the format and establish the version
 */

#include "fi2s.h"
#include "fparameters.h"

const FI2sConfig FI2S_CONFIG_TBL[FI2S_NUM] =
{
#if defined(FI2S0_ID)
    [FI2S0_ID] =
    {
        .instance_id = FI2S0_ID,                  /* i2s id */
        .base_addr = FI2S0_BASE_ADDR,              /* i2s address*/
        .irq_num = FI2S0_IRQ_NUM,                 /* Device interrupt id */
        .irq_prority = 0,
    },
#endif
#if defined(FI2S1_ID)
    [FI2S1_ID] =
    {
        .instance_id = FI2S1_ID,                  /* i2s id */
        .base_addr = FI2S1_BASE_ADDR,              /* i2s address*/
        .irq_num = FI2S1_IRQ_NUM,                 /* Device interrupt id */
        .irq_prority = 0,
    },
#endif
#if defined(FI2S2_ID)
    [FI2S2_ID] =
    {
        .instance_id = FI2S2_ID,                  /* i2s id */
        .base_addr = FI2S2_BASE_ADDR,              /* i2s address*/
        .irq_num = FI2S2_IRQ_NUM,                 /* Device interrupt id */
        .irq_prority = 0,
    },
#endif
#if defined(FI2S3_ID)
    [FI2S3_ID] =
    {
        .instance_id = FI2S3_ID,                  /* i2s id */
        .base_addr = FI2S3_BASE_ADDR,              /* i2s address*/
        .irq_num = FI2S3_IRQ_NUM,                 /* Device interrupt id */
        .irq_prority = 0,
    }
#endif
};
