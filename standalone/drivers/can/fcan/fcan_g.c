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
 * FilePath: fcan_g.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:28:45
 * Description:  This files is for the can default configuration
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2022/5/26  first release
 */


#include "fcan.h"
#include "fparameters.h"

const FCanConfig FCanConfigTbl[FCAN_NUM] =
{
    {
        .instance_id = FCAN0_ID, /* Id of device */
        .base_address = FCAN0_BASE_ADDR,  /* Can base Address */
        .irq_num = FCAN0_IRQ_NUM,
        .caps = FCAN_CAPACITY,
    },
    {
        .instance_id = FCAN1_ID, /* Id of device */
        .base_address = FCAN1_BASE_ADDR,  /* Can base Address */
        .irq_num = FCAN1_IRQ_NUM,
        .caps = FCAN_CAPACITY,
    },
#if defined(FCAN2_ID)
    {
        .instance_id = FCAN2_ID,  /* Id of device */
        .base_address = FCAN2_BASE_ADDR, /* Can base Address */
        .irq_num = FCAN2_IRQNUM,
        .caps = FCAN_CAPACITY,
    }
#endif

};
