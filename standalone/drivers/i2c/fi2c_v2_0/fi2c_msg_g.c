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
 * FilePath: fi2c_msg_g.c
 * Date: 2024-10-1 14:53:42
 * LastEditTime: 2024-10-10 08:36:14
 * Description:  This file is for I2C msg static configuration
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangzq     2024/10/1  first commit
 */


/*  - This file contains a configuration table that specifies the configuration
- 驱动全局变量定义，包括静态配置参数 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fi2c_msg.h"
#include "fparameters.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/**
 * @name: FI2C_MSG_CONFIG_TBL
 * @msg: I2C驱动的默认配置参数
 */
const FI2cMsgConfig FI2C_MSG_CONFIG_TBL[FI2C_MSG_NUM] =
{
    
#if defined(FI2C0_MSG_ID)
    [FI2C0_MSG_ID] =
    {
        .instance_id = FI2C0_MSG_ID,
        .irq_num = FI2C0_MSG_IRQ_NUM,
        .irq_prority = 0,
        .msg =
        {
            .shmem = FI2C0_MSG_SHARE_MEM_ADDR,
            .regfile = FI2C0_MSG_REGFILE_ADDR ,
        }
    },
#endif

#if defined(FI2C1_MSG_ID)
    [FI2C1_MSG_ID] =
    {
        .instance_id = FI2C1_MSG_ID,
        .irq_num = FI2C1_MSG_IRQ_NUM,
        .irq_prority = 0,
        .msg =
        {
            .shmem = FI2C1_MSG_SHARE_MEM_ADDR,
            .regfile = FI2C1_MSG_REGFILE_ADDR ,
        }
    },
#endif

#if defined(FI2C2_MSG_ID)
    [FI2C2_MSG_ID] =
    {
        .instance_id = FI2C2_MSG_ID,
        .irq_num = FI2C2_MSG_IRQ_NUM,
        .irq_prority = 0,
        .msg =
        {
            .shmem = FI2C2_MSG_SHARE_MEM_ADDR,
            .regfile = FI2C2_MSG_REGFILE_ADDR ,
        }
    },
#endif

#if defined(FI2C3_MSG_ID)
    [FI2C3_MSG_ID] =
    {
        .instance_id = FI2C3_MSG_ID,
        .irq_num = FI2C3_MSG_IRQ_NUM,
        .irq_prority = 0,
        .msg =
        {
            .shmem = FI2C3_MSG_SHARE_MEM_ADDR,
            .regfile = FI2C3_MSG_REGFILE_ADDR ,
        },
    },
#endif
};