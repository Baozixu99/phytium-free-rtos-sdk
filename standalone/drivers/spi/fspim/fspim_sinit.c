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
 * FilePath: fspim_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 09:08:24
 * Description:  This file is for providing spim init basic func.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/12/3   init commit
 * 1.1   zhugengyu  2022/4/15   support test mode
 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "fspim.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

extern const FSpimConfig FSPIM_CONFIG_TBL[FSPI_NUM];

/************************** Function Prototypes ******************************/
/**
 * @name: FSpimLookupConfig
 * @msg: 获取FSPIM实例的默认配置
 * @return {const FSpimConfig *} FSPIM实例的默认配置
 * @param {u32} instance_id, SPI控制器ID
 */
const FSpimConfig *FSpimLookupConfig(u32 instance_id)
{
    const FSpimConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FSPI_NUM; index++)
    {
        if (FSPIM_CONFIG_TBL[index].instance_id == instance_id)
        {
            ptr = &FSPIM_CONFIG_TBL[index];
            break;
        }
    }

    return (const FSpimConfig *)ptr;
}
