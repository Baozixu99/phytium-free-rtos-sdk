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
 * FilePath: fwdt_g.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-06-18 09:22:49
 * Description:  This file is for static config of wdt device.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   Wangxiaodong   2021/8/25   init
 * 1.1   Wangxiaodong   2022/6/25   adapt to e2000
 */

#include "fparameters.h"
#include "fwdt.h"

/* default configs of wdt ctrl */
const FWdtConfig FWdtConfigTbl[FWDT_NUM] =
{
    [FWDT0_ID] =
    {
        .instance_id = FWDT0_ID,
        .refresh_base_addr = FWDT0_REFRESH_BASE_ADDR,
        .control_base_addr = FWDT_CONTROL_BASE_ADDR(FWDT0_REFRESH_BASE_ADDR),
        .irq_num = FWDT0_IRQ_NUM,
        .irq_prority = 0,
        .instance_name = "WDT-0"
    },
#if defined(FWDT1_ID)
    [FWDT1_ID] =
    {
        .instance_id = FWDT1_ID,
        .refresh_base_addr = FWDT1_REFRESH_BASE_ADDR,
        .control_base_addr = FWDT_CONTROL_BASE_ADDR(FWDT1_REFRESH_BASE_ADDR),
        .irq_num = FWDT1_IRQ_NUM,
        .irq_prority = 0,
        .instance_name = "WDT-1"
    }
#endif
};
