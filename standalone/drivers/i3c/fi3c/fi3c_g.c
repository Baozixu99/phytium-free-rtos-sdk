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
 * FilePath: fi3c_g.c
 * Date: 2023-9-11 14:53:42
 * LastEditTime: 2023-9-11 08:36:14
 * Description:  This file is for I3C static configuration
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhangyan    2023/9/11  first commit
 */


/*  - This file contains a configuration table that specifies the configuration
- 驱动全局变量定义，包括静态配置参数 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "fi3c.h"
#include "sdkconfig.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/**
 * @name: FI3C_CONFIG_TBL
 * @msg: I3C驱动的默认配置参数
 */
const FI3cConfig FI3C_CONFIG_TBL[FI3C_NUM] =
{
    [FI3C0_ID] =
    {
        .instance_id = FI3C0_ID,
        .base_addr = FI3C0_BASE_ADDR,
        .irq_num = FI3C0_IRQ_NUM,
        .irq_prority = 0,
    },
    [FI3C1_ID] =
    {
        .instance_id = FI3C1_ID,
        .base_addr = FI3C1_BASE_ADDR,
        .irq_num = FI3C1_IRQ_NUM,
        .irq_prority = 0,
    },
#if defined(CONFIG_TARGET_PE220X) 
    [FI3C2_ID] =
    {
        .instance_id = FI3C2_ID,
        .base_addr = FI3C2_BASE_ADDR,
        .irq_num = FI3C2_IRQ_NUM,
        .irq_prority = 0,
    },
    [FI3C3_ID] =
    {
        .instance_id = FI3C3_ID,
        .base_addr = FI3C3_BASE_ADDR,
        .irq_num = FI3C3_IRQ_NUM,
        .irq_prority = 0,
    },
#endif
};
