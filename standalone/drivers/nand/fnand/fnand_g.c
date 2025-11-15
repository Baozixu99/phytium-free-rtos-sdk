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
 * FilePath: fnand_g.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:56:31
 * Description:  This file is for configuration table for devices
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/05/10    first release
 */

#include "fnand.h"
#include "fnand_hw.h"
#include "fparameters.h"


FNandConfig FNandConfigTable[FNAND_NUM] =
{
    {
        .instance_id = FNAND_INSTANCE0,  /* Id of device*/
        .irq_num = FNAND_IRQ_NUM,     /* Irq number */
        .base_address = FNAND_BASE_ADDR,
        .ecc_strength = 8, /* 每次ecc 步骤纠正的位数 */
        .ecc_step_size = 512 /* 进行读写操作时，单次ecc 的步骤的跨度 */
    },
};


