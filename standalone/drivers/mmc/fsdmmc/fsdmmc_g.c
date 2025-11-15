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
 * FilePath: fsdmmc_g.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:53:31
 * Description:  This file contains a configuration table that specifies the configuration of
 * the fsdmmc devices in the system.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/12/2    init
 */

/*  - This file contains a configuration table that specifies the configuration
- 驱动全局变量定义，包括静态配置参数 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"

#include "fsdmmc.h"
#include "fsdmmc_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

const FSdmmcConfig FSDMMC_CONFIG_TBL[FSDMMC_NUM] =
{
    [FSDMMC0_ID] =
    {
        .instance_id = FSDMMC0_ID, /* Id of device*/
        .base_addr   = FSDMMC0_BASE_ADDR,
        .irq_num =
        {
            [FSDMMC_CMD_INTR] = FSDMMC0_CMD_IRQ_NUM,
            [FSDMMC_DMA_BD_INTR] = FSDMMC0_DMA_IRQ_NUM,
            [FSDMMC_ERROR_INTR] = FSDMMC0_ERR_IRQ_NUM
        }
    }
};


/*****************************************************************************/
